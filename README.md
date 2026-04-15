# CRAW

Crawc is a compiler for the craw programming language, a language I wrote a "spec" on 3 years ago when I started getting into rust that I thought would be the "optimal" programming language, it has grown from just a compiler to a tool chain for this language, (well no yet but part of the way there). I would like to say that the language is a non strict, no safety checks, language that has a similar syntax to C/Rust/Zig (3 of my favorite languages to date). Once I finish the CLI, and implement a few more targets for the compiler, I would like to add some more features to the language. Along with hopefully getting a CLI as good as cargo.

## Supported architectures
- ELF32 binary
- x86-64 llvm object file

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
