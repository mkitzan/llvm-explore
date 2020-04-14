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
//		opt -load-pass-plugin=libPrimitiveBranchPass.so -passes=prim-branch-pass ...
//
// License: MIT
//=============================================================================

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// PrimitiveBranchPass implementation
//-----------------------------------------------------------------------------
namespace
{

	bool visitor(Function& F)
	{
		bool Changed{};

		for (auto& BB : F)
		{
			
		}

		return Changed;
	}

	struct PrimitiveBranchPass : PassInfoMixin<PrimitiveBranchPass>
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
llvm::PassPluginLibraryInfo getPrimitiveBranchPassPluginInfo()
{
	return {LLVM_PLUGIN_API_VERSION, "PrimitiveBranchPass", LLVM_VERSION_STRING,
				[](PassBuilder& PB)
				{
					PB.registerPipelineParsingCallback(
						[](StringRef Name, FunctionPassManager &FPM,
								ArrayRef<PassBuilder::PipelineElement>) 
						{
							if (Name == "prim-branch-pass")
							{
								FPM.addPass(PrimitiveBranchPass());
								return true;
							}
							return false;
						});
				}};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize PrimitiveBranchPass when added to the pass pipeline on the
// command line, i.e. via '-passes=prim-branch-pass'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
	return getPrimitiveBranchPassPluginInfo();
}
