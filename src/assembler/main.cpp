#include "assembler.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> [output]" << std::endl;
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string output_file = (argc > 2) ? argv[2] : "a.out";
    
    try {
        // Read source file
        std::ifstream infile(input_file);
        if (!infile) {
            throw std::runtime_error("Cannot open input file: " + input_file);
        }
        
        std::stringstream buffer;
        buffer << infile.rdbuf();
        std::string source = buffer.str();
        
        // Assemble
        Assembler assembler;
        assembler.assemble(source);
        
        // Create ELF
        std::vector<uint8_t> elf = assembler.create_elf();
        
        // Write output
        std::ofstream outfile(output_file, std::ios::binary);
        if (!outfile) {
            throw std::runtime_error("Cannot open output file: " + output_file);
        }
        
        outfile.write(reinterpret_cast<const char*>(elf.data()), elf.size());
        outfile.close();
        
        // Make executable
#ifdef __linux__
        chmod(output_file.c_str(), 0755);
#endif
        
        std::cout << "Assembly successful: " << output_file << std::endl;
        std::cout << "Code size: " << assembler.get_code_size() << " bytes" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
