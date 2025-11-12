#include "assembler.hpp"
#include "mainAssemblerC.h" // extern "C" header

bool assemble_from_string(const char* asm_code, const char* output_file) {
    Assembler assembler;
    std::istringstream input(asm_code);
    std::string line;

    try {
        while (std::getline(input, line)) {
            assembler.assemble_line(line);
        }

        assembler.resolve_labels();
        assembler.write_to_file(output_file);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Assembly failed: " << e.what() << std::endl;
        return false;
    }
}
