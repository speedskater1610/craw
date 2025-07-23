# craw

## how to run and download
#### run
```bash
gcc make.c -o make
./make
```
this will run a script to compile all of the compiler so that you have craw ready to use. 


#### download source code 
you can run 
`git clone https://github.com/speedskater1610/craw` to be able to have the source code for this project


## what diffrent Versions added 
### craw_v1 - craw_v7   
-> set up of the lang how it is parsed and how the file acess works
### craw_v8 -craw_v9   
-> inline asm functions and basic parseing of them ( this include asm std libary which gets turned into diffrent things at run time so you dont have to memorize a bunch of stuff ) example:   
```
#include <stdcraw>

str[100] <hello> = ("hello" 0);

(asm <printHello>) {
    sub rsp, 40 
    lea rcx, <std::fmt::base>
    mov rdx, [*hello]
    xor rax, rax
    call printf
    add rsp, 40 
    ret
}
```   
Where this gets compiled into:   
```
printHello:    
   
    sub rsp, 40    
    lea rcx, [rel compilerVarible_format_no_ln]   
    mov rdx, qword [rel userVarible_hello_point]   
    xor rax, rax   
    call printf   
    add rsp, 40    
    ret
```
this allows for diffrent printing types in the std lib which takes heavy insperatioon on syntax from C++. It also allows for pointers in your asm code which is useful for linking diffrent types of varibles declared outside of the code to the inline asm   
in craw_v9 the std lib and the string (str) varible where added. the str works by   
- str - defines it as a string varible
- [ ... ] size of the varible getting reserved
- < ... > name of the varible
- ( ... ) ->
    - "string getting stored"
    - 0 -> null terminator

 A string gets stored in ASM with 3 main parts ->
 - variable with size
 - pointer
 - accual characters values
   stored something like :
   ```
	userVarible_hello_length db 100
	userVarible_hello_point dq userVarible_hello_chars
	userVarible_hello_chars:
		db 'h', 'e', 'l', 'l', 'o', 0
   ```

### craw_v10 
this added the print and println functions which work by making a temp varible with the string inside like this  
```
msg_count_varible_3_println db 'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', 0
msg_count_varible_4_print db 'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', 0
```
unless you are printing a varible where it saves as   
```
pointer_varible_hello_println dq userVarible_hello_chars
pointer_varible_hello_print dq userVarible_hello_chars
```
which is just a pointer to the varible
and gets printed by

```
	sub rsp, 32
	lea rcx, [rel compilerVarible_format_ln]
	mov rdx, qword [rel pointer_varible_hello_println]
	xor rax, rax
	call printf 
	add rsp, 32
```
you might notice a `compilerVarible_format_ln` varible which and a few others get defined in the .data section of the asm code
```
    compilerVarible_format_ln db "%s", 10, 0
    compilerVarible_format_no_ln db "%s", 0
```



## craw_v11
still in production the only thing changing is the std_compile folder.; formated some code better and tried to add a bunch of new number types and `mut` and `const` to varible types sop you can have `mut str[10] = "hello";` or `const str[10] = "hello";` and consts are declared inside of `.data` and muts are inside of `.bss`
