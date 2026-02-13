# Using the CLI

After you have build the CLI, you can use it to run the assembler or the craw compiler.

- Check that the version is greater that `0.0.0`, if the version is 0.0.0 it can't produce a binary. This caa be done by running
`craw --version` or `craw -v`

## Understanding how the CLI works
The craw commpiler works by using a tag system. To write out the full tag start with the prefix `--` and for shortcuts use the prefix `-`. 

#### tags
- `-h` or `-help`, this will print useful information on diffrent tags.
- `-v` or `--version`, this will print the version of the craw compiler you are using and how you got it
- `-d` or `--debug`, this will put you into debug mode of the compiler, printing useful but compilcated information about your code getting compiler, I only reccomnd using this, if you are familiar with the compilers source code and structure.
- `-a` `--assemble`, this will treat your file as a assembly source file instead of a craw source code file and assemble it using CRASM. 