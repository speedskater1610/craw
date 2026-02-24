# CRAW

CRAW is a compiler and assembler toolchain that is used for compiling a C like language with rust like syntax, It includes 2 binary output options, both are fro x86 linux, a 32 bit assembler backend that includes minimal instructions, but prroduces direct elf binaries that are "smaller", than the other option, a LLVM backend assembler, written in rust, that is the default.

![Progect](https://repobeats.axiom.co/api/embed/5f415f92daddb36bceccae1c71675c76c02de222.svg "Repobeats analytics image")

---

## Getting the CRAW Compiler

1. Clone the repository:
```bash
   git clone https://github.com/speedskater1610/craw
```
3. Compile the compiler:
```bash
make all
```  
   This will generate an executable for the compiler.
   
5. To uninstall the compiler and clean up generated files:
```bash
make clean
```  
   This removes all object files and the compiler executable.

---

## Getting the CRAW Assembler 
### crasm

> Either find the assmebler in releases or make it youself with the following steps : 
1. Clone the repository (if you haven’t already): `git clone https://github.com/speedskater1610/craw`
2. Navigate to the assembler directory: `cd src/assembler/assembler_cpp`
3. Build the assembler: `make -f Assembler.mk`
5. Use the assembler: `./crasm input.asm output`

> **Note:** Currently, the assembler only produces ELF-format binaries.

5. For detailed usage and documentation, see the [Assembler Docs](https://github.com/speedskater1610/craw/tree/main/docs/assembler)
---
