# crasm x86 Assembler

A minimal x86 assembler written in C++ that can assemble a subset of 32-bit x86 instructions and output a runnable ELF executable on Linux.

---

## Table of Contents

- [Features](#features)  
- [Supported Instructions](#supported-instructions)  
- [Operands](#operands)  
- [Usage](#usage)  
- [Classes and Methods](#classes-and-methods)  
  - [Assembler](#assembler-class)  
- [ELF Output](#elf-output)  
- [Example](#example)  

---

## Features

- Supports 32-bit x86 instructions including:
  - Data movement: `mov`, `lea`, `push`, `pop`  
  - Arithmetic: `add`, `sub`, `imul`, `idiv`, `inc`, `dec`  
  - Logic: `xor`, `and`, `or`, `test`  
  - Control flow: `jmp`, `je`, `jne`, `jg`, `jl`, `jge`, `jle`, `call`, `ret`  
  - Shifts: `shl`, `shr`  
  - Interrupts: `int`  
  - No-op: `nop`  

- Resolves labels for jumps and calls.
- Generates a 32-bit ELF executable suitable for Linux.
- Handles immediate values, registers, memory operands, and memory with displacement.

---

## Supported Instructions

| Instruction | Operands | Notes |
|------------|---------|------|
| `mov` | `reg, reg` / `reg, imm` / `reg, [reg]` / `[reg], reg` / with displacement | Standard x86 MOV |
| `add` | `reg, reg` / `reg, imm` | Add values |
| `sub` | `reg, reg` / `reg, imm` | Subtract values |
| `xor` | `reg, reg` | Bitwise XOR |
| `and` | `reg, reg` / `reg, imm` | Bitwise AND |
| `or` | `reg, reg` | Bitwise OR |
| `cmp` | `reg, reg` / `reg, imm` | Compare registers |
| `test` | `reg, reg` | Test bits |
| `inc` | `reg` | Increment register |
| `dec` | `reg` | Decrement register |
| `push` | `reg` / `imm` | Push register or immediate onto stack |
| `pop` | `reg` | Pop from stack into register |
| `call` | `label` / `imm` | Call function |
| `ret` | — | Return from function |
| `jmp` | `label` | Unconditional jump |
| Conditional jumps (`je`, `jne`, `jg`, `jl`, `jge`, `jle`) | `label` | Jump if condition |
| `int` | `imm` | Software interrupt |
| `nop` | — | No operation |
| `lea` | `reg, [reg]` / `reg, [reg + disp]` | Load effective address |
| `imul` | `reg, reg` | Signed multiply |
| `idiv` | `reg` | Signed divide |
| `shl` | `reg, imm` | Shift left |
| `shr` | `reg, imm` | Shift right |

---

## Operands

The assembler supports the following operand types:

- **Registers:** `eax`, `ebx`, `ecx`, `edx`, `esi`, `edi`, `esp`, `ebp`, `al`, `bl`, `cl`, `dl`, `ah`, `bh`, `ch`, `dh`  
- **Immediate values:** Decimal (e.g., `42`) or hexadecimal (e.g., `0x2A`)  
- **Memory references:** `[reg]` or `[reg + imm]`  
- **Labels:** User-defined labels for jumps and calls  

---
