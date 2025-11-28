; this should print `Hi`
_start:
    ; as ascii vals
    push 0x0A21
    push 0x6948

    ; stdout
    mov eax, 4
    mov ebx, 1
    mov ecx, esp   ; pointer to string on stack
    mov edx, 4
    int 0x80

    ; clean up the stack
    add esp, 8

    ; exit
    mov eax, 1
    mov ebx, 0
    int 0x80
