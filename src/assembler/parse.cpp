#include "assembler.hpp"

int32_t Assembler::parse_number(const std::string& str) {
    std::string s = trim(str);

    if (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X") {
        return std::stoi(s, nullptr, 16);
    }

    return std::stoi(s);
}

Assembler::Operand Assembler::parse_operand(const std::string& operand) {
    Operand op;
    std::string op_str = trim(operand);

    // Check for register
    if (registers.find(op_str) != registers.end()) {
        op.type = REG;
        op.reg_val = registers[op_str];
        return op;
    }

    // Check for memory operand
    if (op_str.front() == '[' && op_str.back() == ']') {
        std::string inner = trim(op_str.substr(1, op_str.length() - 2));

        size_t plus_pos = inner.find('+');
        if (plus_pos != std::string::npos) {
            std::string reg_part = trim(inner.substr(0, plus_pos));
            std::string disp_part = trim(inner.substr(plus_pos + 1));

            if (registers.find(reg_part) != registers.end()) {
                op.type = MEM_REG_DISP;
                op.reg_val = registers[reg_part];
                op.disp = parse_number(disp_part);
                return op;
            }
        }

        if (registers.find(inner) != registers.end()) {
            op.type = MEM_REG;
            op.reg_val = registers[inner];
            return op;
        }
    }

    // Check for immediate or label
    if ((op_str[0] >= '0' && op_str[0] <= '9') || op_str[0] == '-' || 
        op_str.substr(0, 2) == "0x") {
        op.type = IMM;
        op.imm_val = parse_number(op_str);
        return op;
    }

    // Must be a label
    op.type = LABEL;
    op.label_name = op_str;
    return op;
}
