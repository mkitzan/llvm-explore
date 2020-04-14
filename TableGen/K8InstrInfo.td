//===-- K8InstrInfo.td - K8 Instruction defs ---------------*- tablegen -*-===//
//
//  Declarations that describe the K8 instruction set
//
//===----------------------------------------------------------------------===//

// Schema for a immediate operand
class Imm<int width> : AsmOperandClass
{
	let Name = "Imm" # width;
	let PredicateMethod = "isImm<" # width # ">";
	let RenderMethod = "addImmOperands";
}

// Def for the allowed 2bit immediate operands
def Imm2 : Operand<i8>
{
	let ParserMatchClass = Imm<2>;
}

// High level schema of a K8 instruction
class K8Instr<bits<4> opcode, string asmstring, dag outs, dag ins> : Instruction
{
	field bits<8> Inst;
	bits<2> rA;
	bits<2> rB = 0b11;

	let Size = 1;

	let Namespace = "K8";
	let DecoderNamespace = "K8";

	let Inst{0-3} = opcode;
	let Inst{4-5} = rA;
	let Inst{6-7} = rB;

	let OutOperandList = outs;
	let InOperandList = ins;
	let AsmString = asmstring;
}

// Specific schemas for K8 insturction archetypes
class K8InstrRR<bits<4> opcode, string asmstring> : K8Instr<opcode, asmstring, (outs GPR:$rA), (ins GPR:$rB)>;
class K8InstrRI<bits<4> opcode, string asmstring> : K8Instr<opcode, asmstring, (outs GPR:$rA), (ins Imm2:$rB)>;
class K8InstrCP<bits<4> opcode, string asmstring> : K8Instr<opcode, asmstring, ?, (ins GPR:$rA, GPR:$rB)>;
class K8InstrR<bits<4> opcode, string asmstring> : K8Instr<opcode, asmstring, (outs GPR:$rA), ?>;

// Defs of concrete instructions
def ST   : K8InstrRR<0b0000, "st\t*$rA, $rB">;
def LD   : K8InstrRR<0b0001, "ld\t$rA, *$rB">;
def ADD  : K8InstrRR<0b0010, "add\trA, rB">;
def SUB  : K8InstrRR<0b0011, "sub\trA, rB">;
def AND  : K8InstrRR<0b0100, "and\trA, rB">;
def OR   : K8InstrRR<0b0101, "or\trA, rB">;
def NOT  : K8InstrRR<0b0110, "not\trA, rB">;
def CLR  : K8InstrRR<0b0111, "clr\trA, rB">;

def NOTx : K8InstrR<0b0110, "not\trA">;
def CLRx : K8InstrR<0b0111, "clr\trA">;

def IFE  : K8InstrCP<0b1000, "ife\trA, rB">;
def IFN  : K8InstrCP<0b1001, "ifn\trA, rB">;
def IFL  : K8InstrCP<0b1010, "ifl\trA, rB">;
def IFG  : K8InstrCP<0b1011, "ifg\trA, rB">;

def INC  : K8InstrRI<0b1100, "inc\trA, rB">;
def DEC  : K8InstrRI<0b1101, "dec\trA, rB">;
def SHL  : K8InstrRI<0b1110, "shl\trA, rB">;
def SHR  : K8InstrRI<0b1111, "shr\trA, rB">;
