# craw

## To get the craw compiler
to get the repository first run 
`git clone "https://github.com/speedskater1610/craw"`.

Next compile the program by running the following: 
`make all`.
This will make a new exacutable for the program.

if for any reason you want to uninstall the craw compiler run
`make clean`, this will remove all of the object files and target exacutable(s)

## how to get the craw assembler, crasm. 
- to get the repository first run `git clone "https://github.com/speedskater1610/craw"`.
- then go into the assembler files `cd /src/assembler/`
- finally use make to build the project `make -f Assembler.mk`
- then you can use the assembler like this `./crasm input.asm output`
- finally refer to the [docs](https://github.com/speedskater1610/craw/tree/main/docs/assembler)
- *Note: This assembler currently only produces ELF-format binaries.*

