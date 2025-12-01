section .data
    hello db 'Hello, World!', 0xA    ; Message with newline
    hello_len equ $ - hello           ; Calculate length

section .text
    global _start

_start:
    ; sys_write(stdout, message, length)
    mov eax, 4              ; syscall number for sys_write
    mov ebx, 1              ; file descriptor 1 (stdout)
    mov ecx, hello          ; pointer to message
    mov edx, hello_len      ; message length
    int 0x80                ; invoke syscall
    
    ; sys_exit(0)
    mov eax, 1              ; syscall number for sys_exit
    xor ebx, ebx            ; exit code 0
    int 0x80                ; invoke syscall
