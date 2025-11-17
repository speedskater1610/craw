    ; How to test - 
    ; `./crasm malloc.asm malloc`
    ; `./malloc`
    ; `echo $?`
        ; this should print 210
        ; which is `1234 % 265`
; test start
_start:
    push 10
    call malloc
    add esp, 4
    mov ebx, eax        ; save pointer in ebx

    ; store a known value (example: 1234)
    mov eax, 1234
    mov [ebx], eax

    ; load it back to eax
    mov eax, [ebx]

    ; exit with that value so we can check it
    mov ebx, eax
    mov eax, 1          ; sys_exit
    int 0x80

; ----------------------------
; malloc:
;   cdecl: caller pushes size (uint32) on stack
;   returns: eax = pointer to allocated block, or 0 on failure
; ----------------------------
malloc:
    push ebp
    mov  ebp, esp

    ; save caller-saved registers we'll use
    push ebx
    push esi
    push edi
    
    ; load requested size from stack [ebp+8]
    mov  ecx, [ebp + 8]     ; ecx = size
    
    ; align size up to 8 bytes: size = (size + 7) & ~7
    add  ecx, 7
    and  ecx, -8
    
    ; --- get current program break: brk(0) ---
    xor  ebx, ebx           ; ebx = 0 (brk(0) request)
    mov  eax, 45            ; syscall number: sys_brk (32-bit)
    int  0x80
    
    ; on return, eax = current_brk (if succeeded)
    mov  esi, eax           ; esi = current_break (old break)
    
    ; compute new break = current_break + size
    mov  ebx, esi           ; ebx = current_break
    add  ebx, ecx           ; ebx = new_break
    
    ; request brk(new_break)
    mov  eax, 45            ; sys_brk
    int  0x80
    
    ; after int 0x80, eax typically contains (implementation dependent)
    ; the kernel's returned break (or error). Check whether it matches ebx.
    cmp  eax, ebx
    jne  malloc_fail       ; if kernel didn't set break to our requested new_break, fail
    
    ; success -> return pointer = old break (esi)
    mov  eax, esi
    
    ; restore registers and return
    pop  edi
    pop  esi
    pop  ebx
    pop  ebp
    ret


malloc_fail:
    xor  eax, eax           ; return 0 on failure
    pop  edi
    pop  esi
    pop  ebx
    pop  ebp
    ret
