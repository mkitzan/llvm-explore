//===-- K8.td - K8 Target defs -----------------------------*- tablegen -*-===//
//
//  Declarations that describe the K8 target machine
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "K8RegisterInfo.td"
include "K8InstrInfo.td"

def K8InstrInfo : InstrInfo;

def K8InstPrinter : AsmWriter
{
	string AsmWriterClassName  = "InstPrinter";
	bit isMCAsmWriter = 1;
}

def K8AsmParser : AsmParser
{
	bit HasMnemonicFirst = 0;
}

def K8 : Target
{
	let InstructionSet = K8InstrInfo;
	let AssemblyWriters = [K8InstPrinter];
	let AssemblyParsers = [K8AsmParser];
}
