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

//-----------------------------------

// for ints 
std::pair<std::string, std::string> parseAndCleanInt(const std::string& input) {
    size_t eqPos = input.find('=');
    if (eqPos == std::string::npos) {
        //no = so error
        std::cout << "ERROR - no \"=\" in line \n\t" << input << std::endl;
    }

    std::string beforeEq = input.substr(0, eqPos);
    std::string afterEq = input.substr(eqPos + 1);

    // remove all spaces
    afterEq.erase(std::remove(afterEq.begin(), afterEq.end(), ' '), afterEq.end());

    return {beforeEq, afterEq};
}


std::string removeComments(const std::string& input) {
    std::istringstream iss(input);
    std::ostringstream oss;
    std::string line;

    while (std::getline(iss, line)) {
        // remove leading space staring with //
        size_t firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace != std::string::npos && line.compare(firstNonSpace, 2, "//") == 0) {
            // ski line if starts with //
            continue;
        }

        // find // and remove anything after it
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line.erase(commentPos);
            // remove tariling spaces (because ima perfectionist)
            //not really just to lazy to get into anything big
            while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
                line.pop_back();
            }
        }

        // add the porcessed line to the out put 
        oss << line << '\n';
    }

    return oss.str();
}



//these few are for checking for certain factors
//check if something starts with another str
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}
//remove all the spaces
std::string remove_spaces(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c != ' ') {
            result += c;
        }
    }
    return result;
}
// extract between angle brackets
std::string extract_angle(const std::string& input) {
    size_t start = input.find('<');
    size_t end = input.find('>');

    if (start != std::string::npos && end != std::string::npos && start < end) {
        return input.substr(start + 1, end - start - 1);
    }

    return ""; // return empty string if not found or invalid
}
std::string extract_parenthesis(const std::string& input) {
    size_t start = input.find('(');
    size_t end = input.find(')');

    if (start != std::string::npos && end != std::string::npos && start < end) {
        return input.substr(start + 1, end - start - 1);
    }

    return ""; // return empty string if not found or invalid
}

std::string extract_curly(const std::string& input) {
    size_t start = input.find('{');
    size_t end = input.rfind('}');

    if (start != std::string::npos && end != std::string::npos && start < end) {
        return input.substr(start + 1, end - start - 1);
    }

    return ""; // return empty string if not found or invalid
}

std::string extract_bracket(const std::string& input) {
    size_t first = input.find('[');
    size_t last = input.rfind(']');

    if (first != std::string::npos && last != std::string::npos && first < last) {
        return input.substr(first + 1, last - first - 1);
    }

    return ""; 
}

std::string extract_quote(const std::string& input) {
    size_t first = input.find('"');
    size_t last = input.rfind('"');

    if (first != std::string::npos && last != std::string::npos && first < last) {
        return input.substr(first + 1, last - first - 1);
    }

    return ""; // Return empty if quotes not found or invalid order
}

std::string replace_pointer_access_asm(const std::string& asmCode) {
    // match [*name]
    std::regex pattern(R"(\[\*(\w+)\])");

    // replace with [userVarible_name_point]
    std::string result = std::regex_replace(asmCode, pattern, "qword [rel userVarible_$1_point]");


    return result;
}
std::string replace_std_asm_acess(const std::string& asmCode) {
    std::string result = asmCode;

    // Replace <std::fmt::base> with compilerVarible_format_no_ln
    result = std::regex_replace(result, std::regex(R"(<std::fmt::base>)"), "[rel compilerVarible_format_no_ln]");

    // Replace <std::fmt::ln> with compilerVarible_format_ln
    result = std::regex_replace(result, std::regex(R"(<std::fmt::ln>)"), "[rel compilerVarible_format_ln]");

    return result;
}
std::string finalize_asm (const std::string& asmCode) {
    return replace_std_asm_acess(replace_pointer_access_asm(asmCode));
}  
std::vector<std::string> split_semi(const std::string& input) {
    std::vector<std::string> result;
    std::string temp;

    for (char ch : input) {
        if (ch == ';') {
            result.push_back(temp);
            temp.clear();
        } else {
            temp += ch;
        }
    }

    // Add the last segment if not empty or if input ends with semicolon
    result.push_back(temp);

    return result;
}

// for spilting the sections up
std::vector<std::string> split_block(const std::string& input) {
    std::vector<std::string> chunks;
    std::istringstream stream(input);
    std::string line;
    std::string block;
    bool insideBlock = false;
    int braceCount = 0;

    while (std::getline(stream, line)) {
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r')) {
            trimmed.pop_back();
        }

        if (!insideBlock && trimmed.find("(asm") != std::string::npos && trimmed.find('{') != std::string::npos) {
            insideBlock = true;
            braceCount = 1;
            block = trimmed + "\n";
            continue;
        }

        if (insideBlock) {
            block += trimmed + "\n";
            for (char c : trimmed) {
                if (c == '{') braceCount++;
                if (c == '}') braceCount--;
            }

            if (braceCount == 0) {
                chunks.push_back(block);
                block.clear();
                insideBlock = false;
            }

            continue;
        }

        if (!trimmed.empty())
            chunks.push_back(trimmed);
    }

    return chunks;
}

std::string format_as_db(const std::string& input) {
    std::string result;

    for (size_t i = 0; i < input.size(); ++i) {
        result += "'";
        result += input[i];
        result += "'";
        if (i != input.size() - 1) {
            result += ", ";
        }
    }

    // Add final comma and 0
    if (!input.empty()) {
        result += ", ";
    }
    result += "0";

    return result;
}

//for compiling the code
std::string compile_logic(std::string input) {
    int printMSGcounter = 0;


    std::string main = 
    // Diffrent parts of the asmmbally file that get edited 
    "section .text\n"
    "    global main\n"
    "main:\n"
    "    push rbp\n"
    "    mov rbp, rsp\n";

    std::string instructions = 
    "   \n";


    //constant varible data
    std::string data = 
    "extern printf\n"
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
                        std::string finalASMprint = "\n\tsub rsp, 32\n\tlea rcx, [rel compilerVarible_format_ln]\n\tmov rdx, ";


                        //what hold the varible name or the value
                        std::string line_data;
                        std::string var_name;
                        //varible name/ pointer to that varible
                        if (lines[x].find("<") != std::string::npos && lines[x].find(">") != std::string::npos) {
                            finalASMprint += "qword [rel ";
                            //variible that gets printed
                            std::string intake_var =  extract_angle(extract_parenthesis(lines[x]));

                            //full line fopr the data sec it is a pointer to what gets printed
                            line_data = ("\n\tpointer_varible_"+intake_var+"_println dq userVarible_"+intake_var+"_chars\n");
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
                        finalASMprint += "xor rax, rax\n\tcall printf \n\tadd rsp, 32\n\n";
                        
                        main += finalASMprint;


                        

                    } 
                    else if (lines[x].find("print") != std::string::npos && lines[x].find("println") == std::string::npos)
                    {
                        //print
                        std::string contains = extract_parenthesis(lines[x]);
                        std::string finalASMprint = "\n\tsub rsp, 32\n\tlea rcx, [rel compilerVarible_format_no_ln]\n\tmov rdx, ";


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
                        finalASMprint += "xor rax, rax\n\tcall printf \n\tadd rsp, 32\n\n";
                        
                        main += finalASMprint;
                    }
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
                if (starts_with(remove_spaces(lines[x]), "size_")) {
                    std::string varible_def = "\n\t";

                    //get the name 
                    std::string varible_name = extract_angle(lines[x]);
                

                    //add the data that they contain
                    auto nameAndValue = parseAndCleanInt(lines[x]);
                    std::string parsedName = extract_angle(remove_spaces(nameAndValue.first));



                    //check for parenthesis in parsed value
                    std::string rawValue = remove_spaces(nameAndValue.second);
                    
                    if (rawValue.find("(") != std::string::npos && rawValue.find(")") != std::string::npos) {
                        rawValue = extract_parenthesis(rawValue);
                    }

                    int parsedValue = std::stoi(rawValue);


                    //compare the varibel anme and parsed name
                    if (varible_name != parsedName) {
                        std::cout << "WARNING with some data types the name must be surroned in < and > this is in relation to : \n\t" << varible_name << " compared to " << parsedName << std::endl;
                    }

                    //used the parsed name so that includeing <> in names arent manditory for all numbers
                    //userVarible_hello_length
                    std::string nameThatGoesIntoASM = ("userVarible_" + parsedName + "size_");

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
                        varible_def += " dt ";
                    }


                    //add the data
                    varible_def += std::to_string(parsedValue);

                    //get a pointer to that data
                    varible_def += "\n\t";
                    varible_def += ("userVarible_" + parsedName + "size_ptr dq " + nameThatGoesIntoASM + "\n\n");


                    userData += varible_def;
                }

                //strings
                if (starts_with(remove_spaces(lines[x]), "str[")) {

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
            }
        }
    }


    // add the user data to there respecvted vvarible placement 
    data += userData;
    bss += userBss;

    //add the instructiosn that the user has to main
    main += instructions;

    std::string output = data + bss + main + end + functions; 

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