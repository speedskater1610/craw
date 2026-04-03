---
name: Progress
about: Report on the current progress of the project
title: "[Progress]"
labels: 
assignees: speedskater1610
---

## Progress

### Completed Stages

- [x] Preprocessor
  - Comment stripping
  - `use "file";` and `use {file};` include directives (inline file contents)
  - String literal expansion (`"hello"` → `{'h','e','l','l','o','\0'}`)
  - `@funcExists<name>` — expands to `1` if `fn name` is defined, else `0`
- [x] Tag system (CLI flags: `-h`, `-v`, `-d`, `-a`, `-S`, `-o`)
- [x] Lexer
  - All keywords: `let fn return if else while goto lbl new this asm use`
  - All types: `i16 i32 i64 u16 u32 u64 f32 f64 char void structinstance defstruct`
  - Type modifiers: `a<type>` (array), `p<type>` (pointer)
  - All operators and delimiters
  - Char literals with escape sequences (`'\n'`, `'\t'`, `'\[65]'`)
  - String and integer/float literals
- [x] Parser (recursive descent)
  - Function definitions with typed parameters and return types
  - `let` variable declarations with optional initialisers
  - `if` / `else if` / `else` chains
  - `while` loops
  - `goto` / `lbl` labels
  - `return` statements
  - `asm {}` inline assembly blocks
  - `use "file"` statements
  - `defstruct` definitions and `structinstance` / `new` expressions
  - All expression precedences (assignment → logical → comparison → bitwise → shift → additive → multiplicative → unary → postfix → primary)
  - Function calls, array indexing, field access, array literals
- [x] AST (27 node kinds, recursive tree with parent/child links, debug printer)
- [x] Code Generator (x86-32, CRASM syntax, ELF32 target)
  - cdecl calling convention (args right-to-left, caller cleans up, return in eax)
  - Stack frame layout (locals at negative ebp offsets, params at +8/+12/…)
  - All arithmetic: `+ - * / %` (with signed `idiv`/`cdq`)
  - All bitwise: `& | ^ ~ << >>`  (shift detects literal count vs variable)
  - All comparisons: `== != < > <= >=` → produce 0/1 in eax
  - Logical: `and or !` with short-circuit compatible 0/1 result
  - Unary: `-` (neg), `~` (not), `!` (logical not)
  - All statement types: let, return, if/else/else-if, while, goto/lbl, asm, expr-stmt
  - Function definitions with correct prologue/epilogue
  - Dead jump elimination (no jmp after return in then-branch)
  - Structs: field layout with alignment, stack allocation, zero-init, field read/write
  - Global variables via esi-relative addressing with callee save/restore
  - String literals inline-pushed on stack with pointer in eax
  - Correct `[ebp - N]` syntax for negative offsets
- [x] Assembler (C++ backend — CRASM)
  - ELF32 output with proper program header
  - All standard x86-32 instructions including: `neg not cdq sar`
  - Negative displacement addressing: `[reg - disp]`
- [x] `make quick` — builds full compiler without Rust/LLVM

### In Progress / Partial
- [ ] `new` heap allocation (currently stack-allocated, `new` keyword accepted but allocates on stack)
- [ ] `this` keyword (lexed, not implemented in parser/codegen)
- [ ] Pointer operations (p<type> variables work but dereference/address-of not codegen'd)
- [ ] Float types (f32/f64 parsed and typed, codegen emits 0 — FPU not in backend)
- [ ] Standard library

### Not Started
- [ ] Codegen optimisation pass
- [ ] Standard library