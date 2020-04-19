//=============================================================================
// FILE:
//		UnusedStorePass.cpp
//
// DESCRIPTION:
//		Prune all store and alloca instructions which have no associated loads to
//		the stored value. Replace all previously stored values with the SSA register
//		value which was originally stored.
//
// USAGE:
//		opt -load libUnusedStorePass.so --legacy-unused-store ...
//		opt -load-pass-plugin=libUnusedStorePass.so -passes=unused-store ...
//
// License: MIT
//=============================================================================

#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// UnusedStorePass implementation
//-----------------------------------------------------------------------------
namespace
{

	bool removeWastedAllocas(BasicBlock& BB)
	{
		bool Changed{};

		for (auto& Inst : make_early_inc_range(BB))
		{
			if (Inst.getOpcode() != Instruction::Alloca || !Inst.user_empty())
			{				
				continue;
			}

			// Prune wasted alloca inst.
			Inst.eraseFromParent();
			Changed = true;
		}

		return Changed;
	}

	bool removeUnusedStores(BasicBlock& BB)
	{
		bool Changed{};

		for (auto& Inst : make_early_inc_range(BB))
		{
			if (Inst.getOpcode() != Instruction::Store)
			{				
				continue;
			}

			bool Redundant{ true };

			for (auto* PtrUser : Inst.getOperand(1)->users())
			{
				// If the store's address is not always stored at, then the store inst
				//	is not redundant. 
				if (cast<Instruction>(PtrUser)->getOpcode() != Instruction::Store)
				{
					Redundant = false;
					break;
				}
			}

			if (Redundant)
			{
				// Prune redundant store inst if found.
				Inst.replaceAllUsesWith(Inst.getOperand(0));
				Inst.eraseFromParent();

				Changed = true;
			}
		}

		return Changed;
	}

	bool visitor(Function& F)
	{
		bool Changed{};

		for (auto& BB : F)
		{
			bool Removed{};

			// First remove an unused store insts.
			Removed |= removeUnusedStores(BB);
			
			// If a store inst was removed, remove its associated alloca.
			if (Removed)
			{
				removeWastedAllocas(BB);
			}

			Changed |= Removed;
		}

		return Changed;
	}

	struct UnusedStorePass : public PassInfoMixin<UnusedStorePass>
	{
		PreservedAnalyses run(Function& F, FunctionAnalysisManager&)
		{
			return (visitor(F) ? PreservedAnalyses::none() : PreservedAnalyses::all());
		}
	};

	struct LegacyUnusedStorePass : public llvm::FunctionPass
	{
		static char ID;

		LegacyUnusedStorePass() : FunctionPass{ ID }
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
llvm::PassPluginLibraryInfo getUnusedStorePassPluginInfo()
{
	return {
		LLVM_PLUGIN_API_VERSION, "UnusedStorePass", LLVM_VERSION_STRING,
		[](PassBuilder& PB)
		{
			PB.registerPipelineParsingCallback(
				[](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) 
				{
					if (Name == "unused-store")
					{
						FPM.addPass(UnusedStorePass());
						return true;
					}
					return false;
				}
			);
		}
	};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize UnusedStorePass when added to the pass pipeline on the
// command line, i.e. via '-passes=unused-store'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return getUnusedStorePassPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyUnusedStorePass::ID = 0;

static RegisterPass<LegacyUnusedStorePass> X(
	"legacy-unused-store", "UnusedStorePass", true, false
);
