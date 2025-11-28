; Constants and equates
BUFFER_SIZE equ 1024
EXIT_SUCCESS equ 0
SYS_EXIT equ 1
SYS_WRITE equ 4

section .data
    ; Variable-size storage directives
    msg db 'Hello, CRASM!', 0xA
    msg_len equ $ - msg
    
    ; Word and dword data
    number dw 42
    bignum dd 0x12345678
    
    ; Array example
    array dd 10, 20, 30, 40, 50

section .bss
    ; Reserved storage
    buffer resb BUFFER_SIZE
    temp_var resd 1
    
    align 16
    aligned_buffer resb 256

section .text
    global _start
    extern printf

_start:
    ; Demonstrate various instruction formats
    
    ; Register-register moves
    mov eax, ebx
    mov ax, bx
    mov al, bl
    
    ; Immediate to register
    mov eax, 42
    mov ebx, BUFFER_SIZE
    mov ecx, number
    
    ; Memory addressing modes
    mov eax, [ebx]
    mov eax, [ebx + 4]
    mov eax, [ebx + ecx*4]
    mov eax, [ebx + ecx*4 + 8]
    mov dword [buffer], 123
    mov word [buffer + 2], 0x1234
    
    ; LEA instruction
    lea eax, [buffer]
    lea ebx, [array + ecx*4]
    
    ; Arithmetic operations
    add eax, ebx
    sub eax, 10
    xor ecx, ecx
    
    ; Stack operations
    push eax
    push ebx
    pop ebx
    pop eax
    
    ; Conditional jumps
    cmp eax, ebx
    je equal
    jne not_equal
    jl less_than
    jg greater_than
    
equal:
    nop
    
not_equal:
    nop
    
less_than:
    nop
    
greater_than:
    nop

.loop:
    ; Short jump example
    dec ecx
    jnz .loop
    
    ; System call to write
    mov eax, SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, msg
    mov edx, msg_len
    int 0x80
    
    ; Exit program
    mov eax, SYS_EXIT
    mov ebx, EXIT_SUCCESS
    int 0x80

; Local label example
local_function:
    push ebp
    mov ebp, esp
    
    ; Function body
    mov eax, [ebp + 8]
    add eax, [ebp + 12]
    
    mov esp, ebp
    pop ebp
    ret
