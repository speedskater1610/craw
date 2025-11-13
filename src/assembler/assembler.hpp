#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>



class Assembler {
public:
    std::map<std::string, uint32_t> labels;
    std::vector<uint8_t> code;
    uint32_t current_address;
    std::map<uint32_t, std::string> label_fixups;
    
    std::map<std::string, int> registers = {
        {"eax", 0}, {"ecx", 1}, {"edx", 2}, {"ebx", 3},
        {"esp", 4}, {"ebp", 5}, {"esi", 6}, {"edi", 7},
        {"al", 0}, {"cl", 1}, {"dl", 2}, {"bl", 3},
        {"ah", 4}, {"ch", 5}, {"dh", 6}, {"bh", 7}
    };
    
    enum OperandType { REG, IMM, MEM_REG, MEM_REG_DISP, LABEL, UNKNOWN };
    
    struct Operand {
        OperandType type;
        int reg_val;
        int32_t imm_val;
        int32_t disp;
        std::string label_name;
    };

    void write_to_file(const char* filename) {
        std::ofstream out(filename, std::ios::binary);
        out.write(reinterpret_cast<const char*>(code.data()), code.size());
        out.close();
    }


    void emit_byte(uint8_t byte);
    void emit_word(uint16_t word);
    void emit_dword(uint32_t dword);
    
    uint8_t modrm_byte(uint8_t mod, uint8_t reg, uint8_t rm);
    
    std::string trim(const std::string& str);
    
    int32_t parse_number(const std::string& str);
    
    Operand parse_operand(const std::string& operand);
    
    void encode_mov(const Operand& dest, const Operand& src);
    void encode_add(const Operand& dest, const Operand& src);
    void encode_sub(const Operand& dest, const Operand& src);
    void encode_xor(const Operand& dest, const Operand& src);
    void encode_and(const Operand& dest, const Operand& src);
    void encode_or(const Operand& dest, const Operand& src);
    void encode_cmp(const Operand& dest, const Operand& src);
    void encode_test(const Operand& dest, const Operand& src);
    void encode_inc(const Operand& op);
    void encode_dec(const Operand& op);
    void encode_push(const Operand& op);
    void encode_pop(const Operand& op);
    void encode_call(const Operand& op);
    void encode_ret();
    void encode_jmp(const Operand& op);
    void encode_conditional_jump(uint8_t opcode, const Operand& op);
    void encode_int(const Operand& op);
    void encode_nop();
    void encode_lea(const Operand& dest, const Operand& src);
    void encode_imul(const Operand& dest, const Operand& src);
    void encode_idiv(const Operand& op);
    void encode_shl(const Operand& dest, const Operand& src);
    void encode_shr(const Operand& dest, const Operand& src);
    
    void assemble_line(const std::string& line);
    
    void resolve_labels();
    
    Assembler() : current_address(0x08048000) {}
    
    void assemble(const std::string& source) {
        std::istringstream iss(source);
        std::string line;
        
        // First pass
        while (std::getline(iss, line)) {
            assemble_line(line);
        }
        
        // Resolve label references
        resolve_labels();
    }
    
    std::vector<uint8_t> create_elf(uint32_t entry_point = 0x08048000) {
        std::vector<uint8_t> elf;
        
        // ELF Header (52 bytes)
        uint8_t elf_header[52] = {
            0x7f, 'E', 'L', 'F',  // Magic
            1,                     // 32-bit
            1,                     // Little endian
            1,                     // ELF version
            0, 0, 0, 0, 0, 0, 0, 0, 0,  // Padding
            2, 0,                  // e_type: ET_EXEC
            3, 0,                  // e_machine: EM_386
            1, 0, 0, 0,           // e_version
        };
        
        elf.insert(elf.end(), elf_header, elf_header + 24);
        
        // e_entry
        elf.push_back(entry_point & 0xFF);
        elf.push_back((entry_point >> 8) & 0xFF);
        elf.push_back((entry_point >> 16) & 0xFF);
        elf.push_back((entry_point >> 24) & 0xFF);
        
        // e_phoff (52 bytes - right after header)
        elf.push_back(52); elf.push_back(0); elf.push_back(0); elf.push_back(0);
        
        // e_shoff (0 - no section headers)
        elf.push_back(0); elf.push_back(0); elf.push_back(0); elf.push_back(0);
        
        // e_flags
        elf.push_back(0); elf.push_back(0); elf.push_back(0); elf.push_back(0);
        
        // e_ehsize (52 bytes)
        elf.push_back(52); elf.push_back(0);
        
        // e_phentsize (32 bytes)
        elf.push_back(32); elf.push_back(0);
        
        // e_phnum (1 program header)
        elf.push_back(1); elf.push_back(0);
        
        // e_shentsize, e_shnum, e_shstrndx (all 0)
        elf.push_back(0); elf.push_back(0);
        elf.push_back(0); elf.push_back(0);
        elf.push_back(0); elf.push_back(0);
        
        // Program Header (32 bytes)
        uint32_t code_offset = 0x1000;
        
        // p_type: PT_LOAD
        elf.push_back(1); elf.push_back(0); elf.push_back(0); elf.push_back(0);
        
        // p_offset
        elf.push_back(code_offset & 0xFF);
        elf.push_back((code_offset >> 8) & 0xFF);
        elf.push_back((code_offset >> 16) & 0xFF);
        elf.push_back((code_offset >> 24) & 0xFF);
        
        // p_vaddr
        elf.push_back(entry_point & 0xFF);
        elf.push_back((entry_point >> 8) & 0xFF);
        elf.push_back((entry_point >> 16) & 0xFF);
        elf.push_back((entry_point >> 24) & 0xFF);
        
        // p_paddr
        elf.push_back(entry_point & 0xFF);
        elf.push_back((entry_point >> 8) & 0xFF);
        elf.push_back((entry_point >> 16) & 0xFF);
        elf.push_back((entry_point >> 24) & 0xFF);
        
        uint32_t code_size = code.size();
        
        // p_filesz
        elf.push_back(code_size & 0xFF);
        elf.push_back((code_size >> 8) & 0xFF);
        elf.push_back((code_size >> 16) & 0xFF);
        elf.push_back((code_size >> 24) & 0xFF);
        
        // p_memsz
        elf.push_back(code_size & 0xFF);
        elf.push_back((code_size >> 8) & 0xFF);
        elf.push_back((code_size >> 16) & 0xFF);
        elf.push_back((code_size >> 24) & 0xFF);
        
        // p_flags (R+X = 5)
        elf.push_back(5); elf.push_back(0); elf.push_back(0); elf.push_back(0);
        
        // p_align
        elf.push_back(0); elf.push_back(0x10); elf.push_back(0); elf.push_back(0);
        
        // Pad to code offset
        while (elf.size() < code_offset) {
            elf.push_back(0);
        }
        
        // Add code
        elf.insert(elf.end(), code.begin(), code.end());
        
        return elf;
    }
    
    size_t get_code_size() const {
        return code.size();
    }
};
