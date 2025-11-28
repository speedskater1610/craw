#include "assembler.hpp"

uint8_t Assembler::modrm_byte(uint8_t mod, uint8_t reg, uint8_t rm) {
    return ((mod & 3) << 6) | ((reg & 7) << 3) | (rm & 7);
}

std::string Assembler::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void Assembler::assemble_line(const std::string& line) {
    std::string l = line;
    size_t comment_pos = l.find(';');
    if (comment_pos != std::string::npos) {
        l = l.substr(0, comment_pos);
    }

    l = trim(l);
    if (l.empty()) return;

    // Check for label
    if (l[l.length() - 1] == ':') {
        std::string label = l.substr(0, l.length() - 1);
        labels[label] = current_address + code.size();
        return;
    }

    // Parse instruction
    std::istringstream iss(l);
    std::string mnemonic;
    iss >> mnemonic;

    std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::tolower);

    std::string rest;
    std::getline(iss, rest);
    std::vector<std::string> operands;

    if (!rest.empty()) {
        std::istringstream operand_stream(rest);
        std::string operand;
        while (std::getline(operand_stream, operand, ',')) {
            operands.push_back(trim(operand));
        }
    }

    // Encode instruction
    if (mnemonic == "mov" && operands.size() == 2) {
        encode_mov(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "add" && operands.size() == 2) {
        encode_add(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "sub" && operands.size() == 2) {
        encode_sub(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "xor" && operands.size() == 2) {
        encode_xor(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "and" && operands.size() == 2) {
        encode_and(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "or" && operands.size() == 2) {
        encode_or(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "cmp" && operands.size() == 2) {
        encode_cmp(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "test" && operands.size() == 2) {
        encode_test(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "inc" && operands.size() == 1) {
        encode_inc(parse_operand(operands[0]));
    } else if (mnemonic == "dec" && operands.size() == 1) {
        encode_dec(parse_operand(operands[0]));
    } else if (mnemonic == "push" && operands.size() == 1) {
        encode_push(parse_operand(operands[0]));
    } else if (mnemonic == "pop" && operands.size() == 1) {
        encode_pop(parse_operand(operands[0]));
    } else if (mnemonic == "call" && operands.size() == 1) {
        encode_call(parse_operand(operands[0]));
    } else if (mnemonic == "ret") {
        encode_ret();
    } else if (mnemonic == "jmp" && operands.size() == 1) {
        encode_jmp(parse_operand(operands[0]));
    } else if (mnemonic == "je" && operands.size() == 1) {
        encode_conditional_jump(0x84, parse_operand(operands[0]));
    } else if (mnemonic == "jne" && operands.size() == 1) {
        encode_conditional_jump(0x85, parse_operand(operands[0]));
    } else if (mnemonic == "jg" && operands.size() == 1) {
        encode_conditional_jump(0x8F, parse_operand(operands[0]));
    } else if (mnemonic == "jl" && operands.size() == 1) {
        encode_conditional_jump(0x8C, parse_operand(operands[0]));
    } else if (mnemonic == "jge" && operands.size() == 1) {
        encode_conditional_jump(0x8D, parse_operand(operands[0]));
    } else if (mnemonic == "jle" && operands.size() == 1) {
        encode_conditional_jump(0x8E, parse_operand(operands[0]));
    } else if (mnemonic == "int" && operands.size() == 1) {
        encode_int(parse_operand(operands[0]));
    } else if (mnemonic == "nop") {
        encode_nop();
    } else if (mnemonic == "lea" && operands.size() == 2) {
        encode_lea(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "imul" && operands.size() == 2) {
        encode_imul(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "idiv" && operands.size() == 1) {
        encode_idiv(parse_operand(operands[0]));
    } else if (mnemonic == "shl" && operands.size() == 2) {
        encode_shl(parse_operand(operands[0]), parse_operand(operands[1]));
    } else if (mnemonic == "shr" && operands.size() == 2) {
        encode_shr(parse_operand(operands[0]), parse_operand(operands[1]));
    } else {
        throw std::runtime_error("Unknown or invalid instruction: " + mnemonic);
    }
}

void Assembler::resolve_labels() {
    for (auto& fixup : label_fixups) {
        uint32_t fixup_pos = fixup.first;
        std::string label = fixup.second;

        if (labels.find(label) == labels.end()) {
            throw std::runtime_error("Undefined label: " + label);
        }

        uint32_t target = labels[label];
        int32_t rel_offset = target - (current_address + fixup_pos + 4);

        code[fixup_pos] = rel_offset & 0xFF;
        code[fixup_pos + 1] = (rel_offset >> 8) & 0xFF;
        code[fixup_pos + 2] = (rel_offset >> 16) & 0xFF;
        code[fixup_pos + 3] = (rel_offset >> 24) & 0xFF;
    }
}
