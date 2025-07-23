#include <iostream>
// for turning back into c 
#include <cstring>
// for removing comments
#include <string>
#include <sstream>
//for splitting into blocks and spilting line by line
#include <vector>
#include <cctype>
//for regex
#include <regex>

//for finding what is in each part of the varibles
#include <utility>
#include <algorithm>

//-----------------------------------
// headers
#include "std.hpp"//logic
#include "std.h"//wrapper

// all of the helper functions
// in helper folder
#include "helper/help.hpp"

//for compiling the code
std::string compile_logic(std::string input) {
    int printMSGcounter = 0;


    std::string main = 
    // Diffrent parts of the asmmbally file that get edited 
    "main:\n"
    "    push rbp\n"
    "    mov rbp, rsp\n";


    std::string text =
    "section .text\n"
    "    global main\n";

    std::string instructions = 
    "   \n";

    std::string externs;

    //constant varible data
    std::string data = 
    "\n"
    "section .data\n"
    "   ; compiler varibles\n"
    "   compilerVarible_format_ln db \"%s\", 10, 0\n"
    "   compilerVarible_format_no_ln db \"%s\", 0\n"
    "   \n"
    "   ; user varibles \n";

    std::string userData = 
    "   \n";


    //varibles that change over time 
    std::string bss = 
    "section .bss\n";


    std::string userBss = 
    "   \n";

    std::string end = 
    "   mov eax, 0\n"
    "   leave\n"
    "   ret\n";

    std::string functions = 
    "\n";


    //if the std lib is here
    bool stdLibary = false;

    //serperate the sections of the input and parse them 
    input = removeComments(input);
    std::vector<std::string> inputs = split_block(input);

    //check for asm functions and just add them first
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (starts_with(remove_spaces(inputs[i]), "(asm")) {
            std::string functionName = extract_angle(inputs[i]);

            //run through all the changinf functions to get the final asm 
            functions += (functionName + ": \n" + finalize_asm(extract_curly(inputs[i])));
        } 
        else { // just the normal code treated as main function
            std::vector<std::string> lines = split_semi(inputs[i]);
            for (size_t x = 0; x < lines.size(); ++x) {
                if (starts_with(remove_spaces(lines[x]), "call")) { //each line
                    main += ("\tcall " + extract_angle(lines[x]) + "\n");
                }

                //what is included
                if (starts_with(remove_spaces(lines[x]), "#include")) {
                    if (lines[x].find("stdcraw") != std::string::npos) {
                        stdLibary = true;


                        // gets included in the file
                        externs += "extern printf\nextern system\n";
                    }
                }

                //printing
                /*
                    sub rsp, 32
                    lea rcx, [rel compilerVarible_format_no_ln]
                    mov rdx, qword [rel pointer_varible_hello_println]
                    xor rax, rax
                    call printf
                    add rsp, 32
                */
                //printMSGcounter  holds a number of how many things have been printed
                //add everything to main
                //println then print
                if(starts_with(remove_spaces(lines[x]), "print")) {
                    printMSGcounter = printMSGcounter + 1;
                    if (lines[x].find("println") != std::string::npos) 
                    {
                        //println
                        std::string contains = extract_parenthesis(lines[x]);
                        std::string finalASMprint = "\n\tsub rsp, 40\n\tlea rcx, [rel compilerVarible_format_ln]\n\tmov rdx, ";


                        //what hold the varible name or the value
                        std::string line_data;
                        std::string var_name;
                        //varible name/ pointer to that varible
                        if (lines[x].find("<") != std::string::npos && lines[x].find(">") != std::string::npos) {
                            finalASMprint += "qword [rel ";
                            //variible that gets printed
                            std::string intake_var =  extract_angle(extract_parenthesis(lines[x]));

                            //full line fopr the data sec it is a pointer to what gets printed
                            line_data = ("\n\tpointer_varible_"+intake_var+"_println_" + std::to_string(printMSGcounter) + " dq userVarible_"+intake_var+"_chars\n");
                            // 
                            var_name = "pointer_varible_"+intake_var+"_println";

                            data += line_data;


                            finalASMprint += var_name;
                            finalASMprint += "]\n\t";
                        } 
                        //thing to print/msg cout should go up
                        else if (lines[x].find("\"") != std::string::npos && lines[x].find("\"") != std::string::npos) {
                            std::string intake_var = extract_quote(extract_parenthesis(lines[x]));

                            //what the printing is
                            line_data = ("\n\tmsg_count_varible_"+(std::to_string(printMSGcounter))+"_println db "+format_as_db(intake_var)+"\n");
                            var_name = ("msg_count_varible_"+(std::to_string(printMSGcounter))+"_println");

                            finalASMprint += (var_name + "\n");

                            data += line_data;
                        }
                        // printMSGcounter holds a number that if what print message that is on
                        // this is only used when a messagfe gets printed 
                        
                        //add it to the asm and wrap everything up
                        finalASMprint += "xor rax, rax\n\tcall printf \n\tadd rsp, 40\n\n";
                        
                        main += finalASMprint;


                        

                    } 
                    else if (lines[x].find("print") != std::string::npos && lines[x].find("println") == std::string::npos)
                    {
                        //print
                        std::string contains = extract_parenthesis(lines[x]);
                        std::string finalASMprint = "\n\tsub rsp, 40\n\tlea rcx, [rel compilerVarible_format_no_ln]\n\tmov rdx, ";


                        //what hold the varible name or the value
                        std::string line_data;
                        std::string var_name;
                        //varible name/ pointer to that varible
                        if (lines[x].find("<") != std::string::npos && lines[x].find(">") != std::string::npos) {
                            finalASMprint += "qword [rel ";
                            //variible that gets printed
                            std::string intake_var =  extract_angle(extract_parenthesis(lines[x]));

                            //full line fopr the data sec it is a pointer to what gets printed
                            line_data = ("\n\tpointer_varible_"+intake_var+"_print dq userVarible_"+intake_var+"_chars\n");
                            // 
                            var_name = "pointer_varible_"+intake_var+"_print";

                            data += line_data;


                            finalASMprint += var_name;
                            finalASMprint += "]\n\t";
                        } 
                        //thing to print/msg cout should go up
                        else if (lines[x].find("\"") != std::string::npos && lines[x].find("\"") != std::string::npos) {
                            std::string intake_var = extract_quote(extract_parenthesis(lines[x]));

                            //what the printing is
                            line_data = ("\n\tmsg_count_varible_"+(std::to_string(printMSGcounter))+"_print db "+format_as_db(intake_var)+"\n");
                            var_name = ("msg_count_varible_"+(std::to_string(printMSGcounter))+"_print");

                            finalASMprint += (var_name + "\n");

                            data += line_data;
                        }
                        // printMSGcounter holds a number that if what print message that is on
                        // this is only used when a messagfe gets printed 
                        
                        //add it to the asm and wrap everything up
                        finalASMprint += "xor rax, rax\n\tcall printf \n\tadd rsp, 40\n\n";
                        
                        main += finalASMprint;
                    }
                } else 

                if (remove_spaces(lines[x]).find("system(") != std::string::npos) {
                    //get and set up the command
                    std::string cmdCommand = extract_parenthesis(extract_quote(lines[x]));
                    std::string cmdVarible = ("cmdCommandVarible" + (std::to_string(printMSGcounter)) + "_" + remove_spaces(cmdCommand) + "");
                    std::string asmVarible = (cmdVarible + " db " + cmdCommand); 
                    data += asmVarible;

                    /*
                    lea rcx, [rel cmd]
                    call system
                    */

                    //run the command
                    std::string cmdMainCommand = "\n\tlea rcx, [rel ";
                    cmdMainCommand += cmdVarible;
                    cmdMainCommand += "]\n\tcall system\n";

                    main += cmdMainCommand;
                }

                //varibles
                //ints 
                /*
                Byte - db  - size_b
                Word - dw - size_w
                Dword - dd - size_d
                Qword - dq - size_q
                Tword -dt - size_t

                int - dd - dword
                float - dq - qword
                */
                //diffrent sizes of ints but these can only hold raw data numbers not the values of other varibles 
                if (starts_with(remove_spaces(lines[x]), "constsize_") || starts_with(remove_spaces(lines[x]), "csize_")) {

                    std::string varible_def = "\n\t";

                    // get the name 
                    std::string varible_name = extract_angle(lines[x]);

                    // add the data that they contain
                    auto nameAndValue = parseAndCleanInt(lines[x]);
                    std::string parsedName = remove_spaces(nameAndValue.first);

                    if (!parsedName.empty() && parsedName.front() == '<' && parsedName.back() == '>') {
                        parsedName = extract_angle(parsedName);
                    }

                    // check for parenthesis in parsed value
                    std::string rawValue = remove_spaces(nameAndValue.second);

                    if (rawValue.find("(") != std::string::npos && rawValue.find(")") != std::string::npos) {
                        rawValue = extract_parenthesis(rawValue);
                    }

                    int parsedValue = std::stoi(rawValue);

                    // compare the variable name and parsed name
                    if (varible_name != parsedName) {
                        std::cout << "WARNING - with some data types the name must be surrounded in < and > this is in relation to : \n\t" << varible_name << " compared to " << parsedName << std::endl;
                    }

                    // build the asm variable name
                    std::string nameThatGoesIntoASM = "userVarible_" + parsedName + "size_";


                    /// get the correct definition for the vaible size
                    if (lines[x].find("size_b") != std::string::npos) {
                        nameThatGoesIntoASM += "b";
                        varible_def += nameThatGoesIntoASM;
                        varible_def += " db ";
                    }
                    else if (lines[x].find("size_w") != std::string::npos) {
                        nameThatGoesIntoASM += "w";
                        varible_def += nameThatGoesIntoASM;
                        varible_def += " dw ";
                    }
                    else if (lines[x].find("size_d") != std::string::npos) {
                        nameThatGoesIntoASM += "d";
                        varible_def += nameThatGoesIntoASM;
                        varible_def += " dd ";
                    }
                    else if (lines[x].find("size_q") != std::string::npos) {
                        nameThatGoesIntoASM += "q";
                        varible_def += nameThatGoesIntoASM;
                        varible_def += " dq ";
                    }
                    else if (lines[x].find("size_t") != std::string::npos) {
                        nameThatGoesIntoASM += "t";
                        varible_def += nameThatGoesIntoASM;
                        varible_def += " dq ";
                    }


                    //add the data
                    varible_def += std::to_string(parsedValue);

                    //get a pointer to that data
                    varible_def += "\n\t";
                    varible_def += ("userVarible_" + parsedName + "size_ptr dq " + nameThatGoesIntoASM + "\n\n");


                    userData += varible_def;
                } else 
                // non const or mutatable 
                /*
db (define byte)	    resb (reserve byte)	1 byte	8-bit value
dw (define word)	    resw (reserve word)	2 bytes	16-bit value
dd (define dword)	    resd (reserve dword)	4 bytes	32-bit value
dq (define qword)	    resq (reserve qword)	8 bytes	64-bit value
dt (define ten bytes)	rest (reserve ten bytes)	10 bytes	80-bit value (x87 float)
dy (define ymmword)	    resy (reserve ymmword)	32 bytes	256-bit value (AVX register)
dz (define zmmword)	    resz (reserve zmmword)	64 bytes	512-bit value (AVX-512 register)

                */
                if (starts_with(remove_spaces(lines[x]), "mutsize_") || starts_with(remove_spaces(lines[x]), "msize_")) {
                    std::string varible_def = "\n\t";

                    //get the name 
                    std::string varible_name = extract_angle(lines[x]);
                

                    //add the data that they contain

                    auto nameAndValue = parseAndCleanInt(lines[x]);
                    std::string parsedName = remove_spaces(nameAndValue.first);

                    if (parsedName.front() == '<' && parsedName.back() == '>') {
                        parsedName = extract_angle(parsedName);
                    }


                    //check for parenthesis in parsed value
                    std::string rawValue = remove_spaces(nameAndValue.second);
                    
                    if (rawValue.find("(") != std::string::npos && rawValue.find(")") != std::string::npos) {
                        rawValue = extract_parenthesis(rawValue);
                    }

                    int parsedValue = std::stoi(rawValue);


                    //compare the varibel anme and parsed name
                    if (varible_name != parsedName) {
                        std::cout << "WARNING - with some data types the name must be surroned in < and > this is in relation to : \n\t" << varible_name << " compared to " << parsedName << std::endl;
                    }

                    //used the parsed name so that includeing <> in names arent manditory for all numbers
                    //userVarible_hello_length
                    std::string nameThatGoesIntoASM = ("userVarible_" + parsedName + "size_");




                    
                    // get the correct definition for the vaible size
                    //lea rax, [rel userVarible_nMBsize_b]
                    if (lines[x].find("size_b") != std::string::npos) {
                        nameThatGoesIntoASM += "b";
                        varible_def += nameThatGoesIntoASM + " resb 1\n";

                        main += "\n\tmov al, " + std::to_string(parsedValue) + "\n\tmov [rel " + nameThatGoesIntoASM + "], al\n";
                    }
                    else if (lines[x].find("size_w") != std::string::npos) {
                        nameThatGoesIntoASM += "w";
                        varible_def += nameThatGoesIntoASM + " resw 1\n";

                        main += "\n\tmov ax, " + std::to_string(parsedValue) + "\n\tmov [rel " + nameThatGoesIntoASM + "], ax\n";
                    }
                    else if (lines[x].find("size_d") != std::string::npos) {
                        nameThatGoesIntoASM += "d";
                        varible_def += nameThatGoesIntoASM + " resd 1\n";

                        main += "\n\tmov eax, " + std::to_string(parsedValue) + "\n\tmov [rel " + nameThatGoesIntoASM + "], eax\n";
                    }
                    else if (lines[x].find("size_q") != std::string::npos) {
                        nameThatGoesIntoASM += "q";
                        varible_def += nameThatGoesIntoASM + " resq 1\n";

                        main += "\n\tmov rax, " + std::to_string(parsedValue) + "\n\tmov [rel " + nameThatGoesIntoASM + "], rax\n";
                    }
                    else if (lines[x].find("size_t") != std::string::npos) {
                        nameThatGoesIntoASM += "t";
                        varible_def += nameThatGoesIntoASM + " resq 1\n";

                        main += "\n\tmov rax, " + std::to_string(parsedValue) + "\n\tmov [rel " + nameThatGoesIntoASM + "], rax\n";
                    }


                    varible_def += "\n\t";

                    //get a pointer to that data
                    userData += ("\n\tuserVarible_" + parsedName + "size_ptr dq " + nameThatGoesIntoASM + "\n\n");

                    userBss += varible_def;

                    // apply the data in main
                }

                //strings
                //constant doesnt change
                if (starts_with(remove_spaces(lines[x]), "cstr") || starts_with(remove_spaces(lines[x]), "conststr") ) {

                    // things about the varible
                    int size = std::stoi(extract_bracket(lines[x]));

                    std::string str_name = extract_angle(lines[x]);
                    std::string str_data = extract_quote(lines[x]);

                    bool nullTerm = true;
                    // check for termination
                    if ((extract_parenthesis(lines[x])).find("0") == std::string::npos) {
                        nullTerm = false;
                    }

                    std::string nullSymbol;
                    if (nullTerm) {
                        nullSymbol = "0";
                    } else {
                        nullSymbol = "";
                    }

                    // turn str_data into the asm format db 'h', 'e', 'l', 'l', 'o', 0
                    std::string chars_asm;
                    for (size_t z = 0; z < size - 1 && z < str_data.size(); ++z) {  // size-1 if size includes null terminator?
                        chars_asm += "'";
                        chars_asm += str_data[z];
                        chars_asm += "', ";
                    }
                    chars_asm += nullSymbol;  // nullSymbol should be "0" or "" 

                    str_data = chars_asm;        

                    std::cout << "str_data: " << str_data << "\n str_data size:" << str_data.size() << std::endl;
                    std::cout << std::endl << "size var: " << size << "\t compare: " << size-1 << std::endl << "-------------\n" << std::endl;
                      
                    if (str_data.find("0") != std::string::npos) {
                        // userVarible_hello_point dq userVarible_hello_chars
                        // userVarible_hello_chars:
                        //         db 'h', 'e', 'l', 'l', 'o', 0
                        userData += remove_spaces("\n\tuserVarible_" + str_name + "_length") + " db " + std::to_string(size) + "\n";
                        userData += remove_spaces("\tuserVarible_" + str_name + "_point") + " dq userVarible_" + str_name + "_chars\n";
                        userData += remove_spaces("\tuserVarible_" + str_name + "_chars:") + "\n\t\tdb " + str_data + "\n\n";

                    }
                    //check ths size for a error
                    if ((str_data.size()) > size * 5) {
                        std::cout << "Warning: string literal for `" << str_name << "` might be too big for the allocated space\n";
                    }
                }

                //string trhat changes
                if (starts_with(remove_spaces(lines[x]), "mstr[") || starts_with(remove_spaces(lines[x]), "mutstr[")) {
                    auto nameAndValue = parseAndCleanStr(lines[x]);
                    std::string parsedName = nameAndValue.first;
                    std::string parseValue = nameAndValue.second;

                    int amount = std::stoi(remove_spaces(extract_bracket(lines[x])));

                    // build the data section init string with quotes and null terminator optionally
                    std::string chars_asm;
                    for (size_t i = 0; i < parseValue.size() && i < (size_t)amount; ++i) {
                        chars_asm += "'";
                        chars_asm += parseValue[i];
                        chars_asm += "', ";
                    }
                    // if amount > parseValue.size() => fill with zeros
                    for (size_t i = parseValue.size(); i < (size_t)amount; ++i) {
                        chars_asm += "0";
                        if (i + 1 < (size_t)amount) chars_asm += ", ";
                    }

                    // def init data variable
                    userData += "\n\tuserVarible_" + parsedName + "_init: db " + chars_asm + "\n";

                    // res un init buffer in BSS
                    userBss += "\n\tuserVarible_" + parsedName + "_main_var resb " + std::to_string(amount) + "\n";

                    // in your main startup code, add code to copy from init to main_var
                    main += "\n\t; Copy initial string for " + parsedName + "\n";
                    main += "\tmov rsi, userVarible_" + parsedName + "_init\n";
                    main += "\tmov rdi, userVarible_" + parsedName + "_main_var\n";
                    main += "\tmov rcx, " + std::to_string(amount) + "\n";
                    main += "\trep movsb\n";
                }
            }
        }
    }


    // add the user data to there respecvted vvarible placement 
    data += userData;
    bss += userBss;

    //add the instructiosn that the user has to main
    main += instructions;

    std::string output = data + bss + text + main + end + functions; 

    output += externs;

    return output;
}


extern "C" char* std_compile(char* fileContents) {
    std::string result = compile_logic(fileContents);



    return strdup(result.c_str());  // caller must free
}



/*
Byte - db  - size_b
Word - dw - size_w
Dword - dd - size_d
Qword - dq - size_q
Tword -dt - size_t

int - dd - dword
float - dq - qword

char - db - byte
pnt - dq - qword  


STRINGS 
str[x] - x is the amount of char/bytes to reserve
they are stored as a char each char and a pointer to the start and end in memory 


defined like this is craw(data)
**indexes start at 0 so there are 5 charters because 0,1,2,3,4 and one more for null terminator
str[5] <hello> = ("hello" 0);

same as 
std::string hello = "hello";

but stored as 

section .data
    userVarible_hello_length db 5
    userVarible_hello_point dq userVarible_hello_chars 

userVarible_hello_chars:
    db 'h', 'e', 'l', 'l', 'o', 0

-------------------------------
so printing works something like
=>

extern printf
section .data
        ;compiler varibles
    compilerVarible_format_ln db "%s", 10, 0
    compilerVarible_format_no_ln db "%s", 0


        ; user varibles
    userVarible_hello_length db 5
    userVarible_hello_point dq userVarible_hello_chars

    userVarible_hello_chars:
        db 'h', 'e', 'l', 'l', 'o', 0

section .bss 


section .text
    global main
main:
    ; standard prologue
    push rbp
    mov rbp, rsp

    ; load printf args
    mov rdi, fmt
    mov rsi, [userVarible_hello_point] 
    xor rax, rax  
    call printf


    ; exit (return 0)
    mov eax, 0
    leave
    ret


*/

/*
str[5] <hello> = ("hello" 0)

(asm <printHello>) {
    mov rdi, fmt
    mov rsi, [*hello] 
    xor rax, rax  
    call printf
}

call <printHello>
*/
