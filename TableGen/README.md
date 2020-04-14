# Fisher Price TableGen

Diving into LLVM's TableGen domain specific language by writing a TableGen target description
for a weird (bad) [8bit assembly language](https://github.com/mkitzan/bare-metal-embedded/tree/master/c/k8-assembly)
I developed awhile back. The architecture has two register instructions, single register instructions, instructions
with immediate arguments. There's also a single register class.

The `llvm-tblgen` tool can grock the TableGen in this folder, but I doubt whether this would integrate into a large
backend for this architecture.

While writing this, TableGen code from the BPF backend was predominately used as a reference.
