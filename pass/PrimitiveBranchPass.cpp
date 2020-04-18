//=============================================================================
// FILE:
//		PrimitiveBranchPass.cpp
//
// DESCRIPTION:
//		Prune all store and alloca instructions which have no associated loads to
//		the stored value. Replace all previously stored values with the SSA register
//		value which was originally stored.
//
// USAGE:
//		opt -load libPrimitiveBranchPass.so --legacy-primitive-branch ...
//		opt -load-pass-plugin=libPrimitiveBranchPass.so -passes=primitive-branch ...
//
// License: MIT
//=============================================================================

#include <functional>
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// PrimitiveBranchPass implementation
//-----------------------------------------------------------------------------
namespace
{
	// Simple HOF to map a function on the insts of a basic block before erasing the block.
	void mapEraseBasicBlock(BasicBlock& Pred, BasicBlock& BB, std::function<void(Instruction&)> Fn)
	{
		BB.removePredecessor(&Pred);

		while (!BB.empty())
		{
			// Fn MUST remove or erase the inst.
			Fn(BB.back());
		}

		BB.eraseFromParent();
	}

	// Convert the primitive branch into a select inst and merge trailing basic block into predecessor.
	void selectConversion(BranchInst* BI, StoreInst* SI, LoadInst* LI, Value* StoreVal)
	{
		// Construct the resulting select inst.
		SelectInst* Select{ SelectInst::Create(BI->getCondition(), SI->getValueOperand(), StoreVal) };

		// Remove primitive branch basic block and erase all its instructions.
		mapEraseBasicBlock(*BI->getParent(), *SI->getParent(),
			[](Instruction& Inst)
			{	
				Inst.replaceAllUsesWith(UndefValue::get(Inst.getType()));
				Inst.eraseFromParent();
			});

		// Bring in the select inst into the basic blocks.
		LI->replaceAllUsesWith(Select);
		Select->insertBefore(BI);

		// Merge the trailing basic block into its predecessor.
		mapEraseBasicBlock(*BI->getParent(), *LI->getParent(),
			[=](Instruction& Inst)
			{
				Inst.removeFromParent();
				Inst.insertAfter(Select);
			});

		// Erase remaining legacy insts.
		LI->eraseFromParent();
		BI->eraseFromParent();
	}

	bool primitiveSuccessor(const BranchInst* BI, BasicBlock* Prim, BasicBlock* Trail, LoadInst** LI)
	{
		// Basic qualities a primitive branch will have.
		StoreInst* SI{ dyn_cast<StoreInst>(Prim->getFirstNonPHI()) };
		bool SizeCheck{ Prim->size() == 2 };
		bool TrailCheck{ Prim->getSingleSuccessor() == Trail };
		bool PredCheck{ BI->getParent() == Prim->getUniquePredecessor() };

		if (SizeCheck && TrailCheck && PredCheck && SI)
		{
			Value* PtrOp{ SI->getPointerOperand() };

			// Assert that the trailing basic block does not store to the store pointer,
			//	and that a load to the store pointer occurs.
			for (auto& Inst : *Trail)
			{
				if (Inst.getOpcode() == Instruction::Store && Inst.getOperand(1) == PtrOp)
				{
					return false;
				}
				else if (Inst.getOpcode() == Instruction::Load && Inst.getOperand(0) == PtrOp)
				{
					*LI = cast<LoadInst>(&Inst);
					return true;
				}
			}
		}

		return false;
	}

	// Gets most recently stored value to PtrOp within a basic block.
	//	Return true if there is a store.
	bool storedValue(BasicBlock const& BB, Value* PtrOp, Value** StoreVal)
	{
		for (auto const& Inst : BB)
		{
			if (Inst.getOpcode() != Instruction::Store)
			{
				continue;
			}

			// Capture last stored value to PtrOp.
			if (Inst.getOperand(1) == PtrOp)
			{
				*StoreVal = Inst.getOperand(0);
			}
		}

		return StoreVal;
	}

	bool visitor(Function& F)
	{
		bool Changed{};

		for (auto& BB : F)
		{
			BranchInst* BI{ dyn_cast<BranchInst>(BB.getTerminator()) };

			if (!BI || BI->isUnconditional())
			{
				continue; 
			}

			BasicBlock* True{ BI->getSuccessor(0) };
			BasicBlock* False{ BI->getSuccessor(1) };
			LoadInst* LI{};
			StoreInst* SI{};
			Value* StoreVal{};

			// Test whether either combination of predecessors matches a primitive branch.
			if (primitiveSuccessor(BI, True, False, &LI))
			{
				SI = dyn_cast<StoreInst>(True->getFirstNonPHI());
			}
			else if (primitiveSuccessor(BI, False, True, &LI))
			{
				SI = dyn_cast<StoreInst>(False->getFirstNonPHI());
			}

			// If an actual primitive branch was matched, convert it to a select inst.
			if (LI && SI && storedValue(BB, SI->getPointerOperand(), &StoreVal))
			{
				selectConversion(BI, SI, LI, StoreVal);
				Changed = true;
			}
		}

		return Changed;
	}

	struct PrimitiveBranchPass : public PassInfoMixin<PrimitiveBranchPass>
	{
		PreservedAnalyses run(Function& F, FunctionAnalysisManager&)
		{
			return (visitor(F) ? PreservedAnalyses::none() : PreservedAnalyses::all());
		}
	};

	struct LegacyPrimitiveBranchPass : public llvm::FunctionPass
	{
		static char ID;

		LegacyPrimitiveBranchPass() : FunctionPass{ ID }
		{}
		bool runOnFunction(llvm::Function &F) override
		{
			return visitor(F);
		}
	};

} // anon namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getPrimitiveBranchPassPluginInfo()
{
	return {
		LLVM_PLUGIN_API_VERSION, "PrimitiveBranchPass", LLVM_VERSION_STRING,
		[](PassBuilder& PB)
		{
			PB.registerPipelineParsingCallback(
				[](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) 
				{
					if (Name == "primitive-branch")
					{
						FPM.addPass(PrimitiveBranchPass());
						return true;
					}
					return false;
				}
			);
		}
	};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize PrimitiveBranchPass when added to the pass pipeline on the
// command line, i.e. via '-passes=primitive-branch'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return getPrimitiveBranchPassPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyPrimitiveBranchPass::ID = 0;

static RegisterPass<LegacyPrimitiveBranchPass> X(
	"legacy-primitive-branch", "PrimitiveBranchPass", true, false
);
