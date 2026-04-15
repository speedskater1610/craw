#include "assembler.hpp"

void Assembler::emit_byte(uint8_t byte) {
    code.push_back(byte);
}
    
void Assembler::emit_word(uint16_t word) {
    code.push_back(word & 0xFF);
    code.push_back((word >> 8) & 0xFF);
}
    
void Assembler::emit_dword(uint32_t dword) {
    code.push_back(dword & 0xFF);
    code.push_back((dword >> 8) & 0xFF);
    code.push_back((dword >> 16) & 0xFF);
    code.push_back((dword >> 24) & 0xFF);
}
