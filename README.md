# CRAW

CRAW is a compiler and assembler toolchain that is used for compiling a C like language with rust like syntax, It includes 2 binary output options, both are for x86 linux, a 32 bit assembler backend that includes minimal instructions, but produces direct elf binaries that are "smaller", than the other option, a LLVM backend assembler, written in rust, that is the default. When using the ELF32 assembler, a single craw source file is used to produce a single assembly and object file.

![Progect](https://repobeats.axiom.co/api/embed/5f415f92daddb36bceccae1c71675c76c02de222.svg "Repobeats analytics image")

---

## Getting the CRAW Assembler 
### crasm

- to build `crawc` with the crasm assembler build with 
```
make quick
```
---



#### Simple Usage
```
# Compile to assembly (inspect output)
./crawc -S -o out.asm tests/test_comprehensive.craw

# Compile to ELF32 binary
./crawc -o out tests/test_comprehensive.craw

# Debug mode (prints AST and preprocessed source)
./crawc -d -S -o out.asm tests/test1.craw
```
