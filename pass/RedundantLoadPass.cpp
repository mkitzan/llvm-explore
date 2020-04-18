//=============================================================================
// FILE:
//		RedundantLoadPass.cpp
//
// DESCRIPTION:
//		Prune all load instructions which load from a pointer whose value was stored
//		within the basic block. Replace all previously loaded values with the SSA
//		register value which was originally stored.
//
// USAGE:
//		opt -load libRedundantLoadPass.so --legacy-redundant-load ...
//		opt -load-pass-plugin=libRedundantLoadPass.so -passes=redundant-load ...
//
// License: MIT
//=============================================================================

#include <unordered_map>
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using ValuePtrMap = std::unordered_map<Value*, Value*>;

//-----------------------------------------------------------------------------
// RedundantLoadPass implementation
//-----------------------------------------------------------------------------
namespace
{

	bool elideRedundantLoads(BasicBlock& BB, ValuePtrMap& Pointers)
	{
		bool Changed{};

		for (auto& Inst : make_early_inc_range(BB))
		{
			if (Inst.getOpcode() == Instruction::Store)
			{				
				// Cache the pointer and data operands of every store inst.
				Pointers[Inst.getOperand(1)] = Inst.getOperand(0);
			}
			else if (Inst.getOpcode() == Instruction::Load)
			{
				auto Pair{ Pointers.find(Inst.getOperand(0)) };

				// Replace every load inst whose data was set as a store inst
				//	within this basic block with the SSA register value of the store.
				//	Prune the redundant load inst from the basic block.
				if (Pair != Pointers.end())
				{
					Inst.replaceAllUsesWith(Pair->second);
					Inst.eraseFromParent();
					Changed = true;
				}
			}
		}

		return Changed;
	}

	bool visitor(Function& F)
	{
		bool Changed{};
		std::unordered_map<Value*, Value*> Pointers{};

		for (auto& BB : F)
		{
			Pointers.clear();
			Changed |= elideRedundantLoads(BB, Pointers);
		}

		return Changed;
	}

	struct RedundantLoadPass : public PassInfoMixin<RedundantLoadPass>
	{
		PreservedAnalyses run(Function& F, FunctionAnalysisManager&)
		{
			return (visitor(F) ? PreservedAnalyses::none() : PreservedAnalyses::all());
		}
	};

	struct LegacyRedundantLoadPass : public llvm::FunctionPass
	{
		static char ID;

		LegacyRedundantLoadPass() : FunctionPass{ ID }
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
llvm::PassPluginLibraryInfo getRedundantLoadPassPluginInfo()
{
	return {
		LLVM_PLUGIN_API_VERSION, "RedundantLoadPass", LLVM_VERSION_STRING,
		[](PassBuilder& PB)
		{
			PB.registerPipelineParsingCallback(
				[](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) 
				{
					if (Name == "redundant-load")
					{
						FPM.addPass(RedundantLoadPass());
						return true;
					}
					return false;
				}
			);
		}
	};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize RedundantLoadPass when added to the pass pipeline on the
// command line, i.e. via '-passes=redundant-load'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return getRedundantLoadPassPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyRedundantLoadPass::ID = 0;

static RegisterPass<LegacyRedundantLoadPass> X(
	"legacy-redundant-load", "RedundantLoadPass", true, false
);
