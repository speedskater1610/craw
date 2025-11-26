# CRASM x86 Assembler
A lightweight x86_32 assembler for Manjaro Linux that generates ELF32 executables.
## Resources 
- [user manual](https://github.com/speedskater1610/craw/edit/main/docs/assembler/user_manual.md)
- [build the CRASM assembler](https://github.com/speedskater1610/craw/blob/main/docs/assembler/build.md)
- [features *Majority of documentation*](https://github.com/speedskater1610/craw/blob/main/docs/assembler/features.md)

## Table of Contents
- [Installation](#installation)
- [Usage](#usage)
- [Supported Instructions](#supported-instructions)
- [Operand Types](#operand-types)
- [Features](#features)
- [Limitations](#limitations)
- [Examples](#examples)

---

## Installation

### Compile the Assembler
1. First clone the repo `git clone https://github.com/speedskater1610/craw` *NOTE: This will clone the entire project so you will also have acess to the craw compiler - but this guide will only cover how to build the CRASM assembler*
2. Then go into the assembler folder, `cd /src/assembler/`. This will put you into the folder of all of the source code for the CRASM assembler.
3. Verify that everything worked, `ls`, you should see at least the following files: `Assembler.mk`, `assembler.cpp`, `assembler.hpp`, `emit.cpp`, `encode.cpp`, `main.cpp`, `parse.cpp`. *NOTE: you may also see the files, `mainAssembler.cpp` and `mainAssemblerC.h`, but they arent need for the stand alone CRASM assembler, they are for the compiler to talk with the assembler*
4. Finally make the project using the [makefile](https://github.com/speedskater1610/craw/blob/main/src/assembler/Assembler.mk), `make -f Assembler.mk`
5. Then review [CRASM assembler user manual](https://github.com/speedskater1610/craw/blob/main/docs/assembler/user_manual.md) or [CRASM assembler documentation](https://github.com/speedskater1610/craw/tree/main/docs/assembler)

*If you would like to unistall the binaries and object files after making the project you can run `make -f Assembler.mk clean`*

### Requirements
- G++ compiler with C++11 support
- Linux system (Manjaro, Ubuntu, Debian, etc.)
- 32-bit execution support (may need `libc6-i386` on 64-bit systems)

---

## Usage

### Basic Command

```bash
./crasm input.asm [output]
```

- `input.asm` - Source assembly file
- `output` - Optional output filename (default: `a.out`)

### Example

```bash
./crasm hello.asm hello
./hello
```

---

## Supported Instructions

### Data Movement

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `mov` | reg, imm | Move immediate to register | `mov eax, 42` |
| `mov` | reg, reg | Move register to register | `mov ebx, eax` |
| `mov` | [reg], reg | Move register to memory | `mov [ebx], eax` |
| `mov` | reg, [reg] | Move memory to register | `mov eax, [ebx]` |
| `mov` | [reg+disp], reg | Move with displacement | `mov [ebp + 8], eax` |
| `mov` | reg, [reg+disp] | Load with displacement | `mov eax, [ebp + 8]` |
| `lea` | reg, [reg] | Load effective address | `lea eax, [ebx]` |
| `lea` | reg, [reg+disp] | Load address with offset | `lea ecx, [ebp + 8]` |

### Arithmetic Operations

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `add` | reg, imm | Add immediate to register | `add eax, 10` |
| `add` | reg, reg | Add register to register | `add eax, ebx` |
| `sub` | reg, imm | Subtract immediate from register | `sub eax, 5` |
| `sub` | reg, reg | Subtract register from register | `sub eax, ebx` |
| `inc` | reg | Increment register | `inc eax` |
| `dec` | reg | Decrement register | `dec ebx` |
| `imul` | reg, reg | Signed multiply | `imul eax, ebx` |
| `idiv` | reg | Signed divide (eax by operand) | `idiv ebx` |

### Logical Operations

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `and` | reg, reg | Bitwise AND | `and eax, ebx` |
| `and` | reg, imm | AND with immediate | `and eax, 0xFF` |
| `or` | reg, reg | Bitwise OR | `or eax, ebx` |
| `xor` | reg, reg | Bitwise XOR | `xor eax, eax` |
| `test` | reg, reg | Logical compare (sets flags) | `test eax, eax` |
| `cmp` | reg, imm | Compare register with immediate | `cmp eax, 0` |
| `cmp` | reg, reg | Compare register with register | `cmp eax, ebx` |

### Bit Shifting

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `shl` | reg, imm | Shift left | `shl eax, 2` |
| `shr` | reg, imm | Shift right | `shr ebx, 1` |

### Stack Operations

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `push` | reg | Push register onto stack | `push eax` |
| `push` | imm | Push immediate onto stack | `push 42` |
| `pop` | reg | Pop from stack to register | `pop eax` |

### Control Flow

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `call` | label | Call subroutine | `call malloc` |
| `ret` | - | Return from subroutine | `ret` |
| `jmp` | label | Unconditional jump | `jmp loop_start` |
| `je` | label | Jump if equal (ZF=1) | `je equal_label` |
| `jne` | label | Jump if not equal (ZF=0) | `jne not_equal` |
| `jg` | label | Jump if greater | `jg greater_label` |
| `jl` | label | Jump if less | `jl less_label` |
| `jge` | label | Jump if greater or equal | `jge ge_label` |
| `jle` | label | Jump if less or equal | `jle le_label` |

### System & Misc

| Instruction | Operands | Description | Example |
|------------|----------|-------------|---------|
| `int` | imm | Software interrupt | `int 0x80` |
| `nop` | - | No operation | `nop` |

---

## Operand Types

### Registers (32-bit)
- **General Purpose**: `eax`, `ebx`, `ecx`, `edx`
- **Index/Pointer**: `esi`, `edi`, `esp`, `ebp`

### Registers (8-bit)
- **Low byte**: `al`, `bl`, `cl`, `dl`
- **High byte**: `ah`, `bh`, `ch`, `dh`

### Immediate Values
- **Decimal**: `42`, `-10`
- **Hexadecimal**: `0x2A`, `0xFF`
- **Negative**: `-1`, `-100`

### Memory Addressing
- **Direct register**: `[eax]`, `[ebx]`
- **Register + displacement**: `[ebp + 8]`, `[esp - 4]`

### Labels
- Used for jumps and calls
- Must end with `:` when defined
- Example: `loop_start:`, `exit_fail:`

---

## Features

### Two-Pass Assembly
1. **First Pass**: Collects all labels and their addresses
2. **Second Pass**: Resolves label references and generates machine code

### ELF32 Generation
- Creates valid 32-bit ELF executables
- Sets proper entry point (`0x08048000` by default)
- Configures program headers for executable text segment
- Automatically sets executable permissions (`chmod 755`)

### Label Support
- Forward references (jump to labels defined later)
- Backward references (jump to labels defined earlier)
- Automatic address resolution

### Comments
```asm
; This is a comment
mov eax, 42    ; inline comment
```

---

## Limitations

### Not Supported

#### Memory Operations
- ❌ Direct immediate to memory: `mov [eax], 42`
  - **Workaround**: Use register intermediary
  ```asm
  mov ebx, 42
  mov [eax], ebx
  ```

- ❌ Size specifiers: `mov byte [eax], 0x41`, `mov dword [eax], 42`
  - **Workaround**: Use appropriate register size
  ```asm
  mov al, 0x41      ; For byte operations
  mov [eax], al
  ```

- ❌ Memory to memory moves: `mov [eax], [ebx]`
  - **Workaround**: Use register intermediary
  ```asm
  mov ecx, [ebx]
  mov [eax], ecx
  ```

#### Jump Instructions
- ❌ `jz` (jump if zero) - Use `je` instead
- ❌ `jnz` (jump if not zero) - Use `jne` instead

#### Advanced Features
- ❌ Data sections (`.data`, `.bss`)
- ❌ Section directives
- ❌ Macros
- ❌ Local labels (`.label`)
  - **Workaround**: Use unique global labels
- ❌ String literals
- ❌ `db`, `dw`, `dd` directives

#### Instructions Not Implemented
- Floating point (FPU) instructions
- SSE/AVX instructions
- Some arithmetic: `mul`, `div` (use `imul`, `idiv`)
- String operations: `movs`, `cmps`, `scas`, etc.
- Advanced control flow: `loop`, `jecxz`

---

## Examples

### Hello World (Simplified)

```asm
_start:
    ; Push "Hi!\n" onto stack in reverse order
    push 0x0A21         ; '\n' and '!'
    push 0x6948         ; 'H' and 'i'
    
    ; Write to stdout
    mov eax, 4          ; sys_write
    mov ebx, 1          ; stdout
    mov ecx, esp        ; pointer to string on stack
    mov edx, 4          ; length
    int 0x80
    
    ; Clean up stack
    add esp, 8
    
    ; Exit
    mov eax, 1          ; sys_exit
    mov ebx, 0          ; exit code
    int 0x80
```

### Simple Exit

```asm
_start:
    mov eax, 1          ; sys_exit
    mov ebx, 42         ; exit code
    int 0x80
```

### Function Call with Stack Frame

```asm
_start:
    push 10             ; argument
    call my_function
    add esp, 4          ; clean up stack
    
    mov ebx, eax        ; use return value
    mov eax, 1          ; sys_exit
    int 0x80

my_function:
    push ebp
    mov ebp, esp
    
    ; Access argument at [ebp + 8]
    mov eax, [ebp + 8]
    add eax, 5          ; do something
    
    pop ebp
    ret
```

### Loop Example

```asm
_start:
    mov ecx, 10         ; counter

loop_start:
    dec ecx
    test ecx, ecx
    jne loop_start      ; loop while ecx != 0
    
    mov eax, 1
    mov ebx, 0
    int 0x80
```

### Conditional Logic

```asm
_start:
    mov eax, 5
    mov ebx, 10
    
    cmp eax, ebx
    jge eax_greater_or_equal
    
    ; eax < ebx
    mov ecx, 1
    jmp done
    
eax_greater_or_equal:
    ; eax >= ebx
    mov ecx, 0
    
done:
    mov eax, 1
    mov ebx, ecx
    int 0x80
```

### Using Malloc

```asm
_start:
    push 100            ; allocate 100 bytes
    call malloc
    add esp, 4
    
    test eax, eax       ; check if null
    je allocation_failed
    
    ; Use allocated memory (pointer in eax)
    mov ebx, eax
    mov ecx, 42
    mov [ebx], ecx
    
    mov eax, 1
    mov ebx, 0
    int 0x80

allocation_failed:
    mov eax, 1
    mov ebx, 1
    int 0x80
```

---

## Linux System Calls

Common syscalls used in assembly (x86_32):

| Syscall | EAX | EBX | ECX | EDX | Description |
|---------|-----|-----|-----|-----|-------------|
| sys_exit | 1 | exit_code | - | - | Exit program |
| sys_write | 4 | fd | buffer* | count | Write to file |
| sys_read | 3 | fd | buffer* | count | Read from file |
| sys_brk | 45 | addr | - | - | Change data segment size |

### Example: Write System Call

```asm
mov eax, 4          ; sys_write
mov ebx, 1          ; stdout (fd = 1)
mov ecx, buffer     ; pointer to data
mov edx, length     ; number of bytes
int 0x80            ; invoke syscall
```

---

## Tips & Best Practices

### Memory Alignment
The assembler automatically aligns memory allocations to 8-byte boundaries for optimal performance.

### Stack Management
Always clean up the stack after function calls:
```asm
push arg1
push arg2
call function
add esp, 8          ; clean up 2 arguments (4 bytes each)
```

### Register Preservation
Preserve caller-saved registers in functions:
```asm
function:
    push ebp
    mov ebp, esp
    push ebx        ; preserve
    push esi        ; preserve
    
    ; ... function body ...
    
    pop esi         ; restore
    pop ebx         ; restore
    pop ebp
    ret
```

### Error Checking
Always check return values from system calls and functions:
```asm
call malloc
test eax, eax       ; check for null
je error_handler
```

### Comments
Document your assembly code thoroughly:
```asm
; Calculate factorial of 5
mov eax, 5          ; n = 5
mov ebx, 1          ; result = 1
```

---

## Troubleshooting

### Common Errors

**"Unknown instruction"**
- Check spelling and instruction name
- Verify instruction is in supported list

**"Unsupported operands"**
- Check operand types match instruction requirements
- Use workarounds for unsupported combinations

**"Undefined label"**
- Ensure label is defined with `:` suffix
- Check spelling matches exactly

**Segmentation Fault**
- Check memory addresses are valid
- Verify malloc succeeded before dereferencing
- Ensure proper stack frame setup

**Exit Code Issues**
- Remember exit codes are 8-bit (0-255)
- Values > 255 will be truncated: `1234 % 256 = 210`

---

## Building Standard Library

Create reusable functions in separate files:

```asm
; stdlib/malloc.asm
malloc:
    push ebp
    mov ebp, esp
    ; ... implementation ...
    pop ebp
    ret
```

Include by concatenating files:
```bash
cat stdlib/*.asm main.asm > program.asm
./crasm program.asm program
```
*NOTE: make sure that you are in the craw/src/assembler/ directory*
