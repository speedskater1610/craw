# CRAW

CRAW is a compiler and assembler toolchain designed to simplify programming and assembly for ELF binaries.
![Progect](https://repobeats.axiom.co/api/embed/5f415f92daddb36bceccae1c71675c76c02de222.svg "Repobeats analytics image")
## *NOTE THE CRASM assemnbler in `src/assembler/assembler/` is what this branch is made to work on since it doesn't work feel free to contribute to it if you know any rust or haskell.*
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
### crasm_cpp

1. Clone the repository (if you haven’t already): `git clone https://github.com/speedskater1610/craw`
2. Navigate to the assembler directory: `cd src/assembler/assembler_cpp`
3. Build the assembler: `make -f Assembler.mk`
4. Use the assembler: `./crasm input.asm output`

> **Note:** Currently, the assembler only produces ELF-format binaries.

5. For detailed usage and documentation, see the [Assembler Docs](https://github.com/speedskater1610/craw/tree/main/docs/assembler)
*IN the future I will write a more complete assembler in rust or haskell and use that this is more of a toy assembler rather than anything that will be used*

---

### crasm
*This can be built from the make file in src/assembler/assembler/Makefile*
1. Clone the repository (if you haven’t already): `git clone https://github.com/speedskater1610/craw` then go into the repo `cd craw`
2. Navigate to the assembler directory: `cd src/assembler/assembler`
3. Build the assembler: `make`
4. make it a global variable `sudo mv crasm /usr/local/bin/`
5. Use the assembler: `crasm input.asm output`
6. Link the resulting object file `ld -m elf_i386 output.o -o output`
