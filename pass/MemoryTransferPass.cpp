//=============================================================================
// FILE:
//		MemoryTranferPass.cpp
//
// DESCRIPTION:
//		Prune all store / load pairs which brace the transitions between basic blocks
//		local to a single function where the loading basic block has a single predecessor.
//
// USAGE:
//		opt -load-pass-plugin=libMemoryTranferPass.so -passes=memory-transfer-pass ...
//
// License: MIT
//=============================================================================

#include <unordered_map>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// MemoryTranferPass implementation
//-----------------------------------------------------------------------------
namespace
{
	bool elideRedundantLoads(BasicBlock* BB, std::unordered_map<Value*, Value*> const& Pointers)
	{
		bool Changed{};

		for (auto& Inst : make_early_inc_range(*BB))
		{
			if (Inst.getOpcode() == Instruction::Load)
			{
				auto Pair{ Pointers.find(Inst.getOperand(0)) };

				// Replace every load inst whose data was set as a store inst
				// within this basic block with the SSA register value of the store.
				// Prune the redundant load inst from the basic block.
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

	bool elideMemoryTransfers(BasicBlock& BB)
	{
		bool Changed{};
		std::unordered_map<Value*, Value*> Pointers{};

		for (auto& Inst : BB)
		{
			if (Inst.getOpcode() == Instruction::Store)
			{				
				// Cache the pointer and data operands of every store inst.
				Pointers[Inst.getOperand(1)] = Inst.getOperand(0);
			}
			else if (Inst.getOpcode() == Instruction::Br)
			{
				auto& Branch{ cast<BranchInst>(Inst) };

				// Prune memory transfer loads to successors basic blocks with a single predecessor
				for (auto* Successor : Branch.successors())
				{
					if (Successor->getSinglePredecessor())
					{
						Changed |= elideRedundantLoads(Successor, Pointers);
					}
				}
			}
		}

		return Changed;
	}

	bool visitor(Function& F)
	{
		bool Changed{};

		for (auto& BB : F)
		{
			Changed |= elideMemoryTransfers(BB);
		}

		return Changed;
	}

	struct MemoryTranferPass : PassInfoMixin<MemoryTranferPass>
	{
		PreservedAnalyses run(Function& F, FunctionAnalysisManager&)
		{
			return (visitor(F) ? PreservedAnalyses::none() : PreservedAnalyses::all());
		}
	};

}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getMemoryTranferPassPluginInfo()
{
	return {LLVM_PLUGIN_API_VERSION, "MemoryTranferPass", LLVM_VERSION_STRING,
					[](PassBuilder& PB)
					{
						PB.registerPipelineParsingCallback(
							[](StringRef Name, FunctionPassManager &FPM,
									ArrayRef<PassBuilder::PipelineElement>) 
							{
								if (Name == "memory-transfer-pass")
								{
									FPM.addPass(MemoryTranferPass());
									return true;
								}
								return false;
							});
					}};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize MemoryTranferPass when added to the pass pipeline on the
// command line, i.e. via '-passes=memory-transfer-pass'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return getMemoryTranferPassPluginInfo();
}
