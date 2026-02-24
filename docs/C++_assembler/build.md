# How to Build the CRASM assembler
1. First clone the repo `git clone https://github.com/speedskater1610/craw` *NOTE: This will clone the entire project so you will also have acess to the craw compiler - but this guide will only cover how to build the CRASM assembler*
2. Then go into the assembler folder, `cd /src/assembler/`. This will put you into the folder of all of the source code for the CRASM assembler.
3. Verify that everything worked, `ls`, you should see at least the following files: `Assembler.mk`, `assembler.cpp`, `assembler.hpp`, `emit.cpp`, `encode.cpp`, `main.cpp`, `parse.cpp`. *NOTE: you may also see the files, `mainAssembler.cpp` and `mainAssemblerC.h`, but they arent need for the stand alone CRASM assembler, they are for the compiler to talk with the assembler*
4. Finally make the project using the [makefile](https://github.com/speedskater1610/craw/blob/main/src/assembler/Assembler.mk), `make -f Assembler.mk`
5. Then review [CRASM assembler user manual](https://github.com/speedskater1610/craw/blob/main/docs/assembler/user_manual.md) or [CRASM assembler documentation](https://github.com/speedskater1610/craw/tree/main/docs/assembler)

*If you would like to unistall the binaries and object files after making the project you can run `make -f Assembler.mk clean`*
