//===-- K8RegisterInfo.td - K8 Register defs ---------------*- tablegen -*-===//
//
//  Declarations that describe the K8 register file
//
//===----------------------------------------------------------------------===//

// Schema for a register
class Ri<bits<2> encoding, string name> : Register<name>
{
	bits<2> Encoding = encoding;
	let Namespace = "K8";
}

// Defs of concrete registers
foreach I = 0-2 in
{
	def R#I : Ri<I, "r"#I>;
}

def PC : Ri<3, "pc">;

// Def of entire register class
def GPR : RegisterClass<"K8", [i8], 1, (add (sequence "R%u", 0, 2), PC)>;
