#include "assembler.hpp"

void Assembler::encode_mov(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        Assembler::emit_byte(0xB8 + dest.reg_val);
        Assembler::emit_dword(src.imm_val);
    } else if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x89);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else if (dest.type == MEM_REG && src.type == REG) {
        Assembler::emit_byte(0x89);
        Assembler::emit_byte(modrm_byte(0, src.reg_val, dest.reg_val));
    } else if (dest.type == REG && src.type == MEM_REG) {
        Assembler::emit_byte(0x8B);
        Assembler::emit_byte(modrm_byte(0, dest.reg_val, src.reg_val));
    } else if (dest.type == MEM_REG_DISP && src.type == REG) {
        Assembler::emit_byte(0x89);
        Assembler::emit_byte(modrm_byte(2, src.reg_val, dest.reg_val));
        Assembler::emit_dword(dest.disp);
    } else if (dest.type == REG && src.type == MEM_REG_DISP) {
        Assembler::emit_byte(0x8B);
        Assembler::emit_byte(modrm_byte(2, dest.reg_val, src.reg_val));
        Assembler::emit_dword(src.disp);
    } else {
        throw std::runtime_error("Unsupported MOV operands");
    }
}

void Assembler::encode_add(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        if (dest.reg_val == 0) {
            Assembler::emit_byte(0x05);
            Assembler::emit_dword(src.imm_val);
        } else {
            Assembler::emit_byte(0x81);
            Assembler::emit_byte(modrm_byte(3, 0, dest.reg_val));
            Assembler::emit_dword(src.imm_val);
        }
    } else if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x01);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported ADD operands");
    }
}

void Assembler::encode_sub(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        if (dest.reg_val == 0) {
            Assembler::emit_byte(0x2D);
            Assembler::emit_dword(src.imm_val);
        } else {
            Assembler::emit_byte(0x81);
            Assembler::emit_byte(modrm_byte(3, 5, dest.reg_val));
            Assembler::emit_dword(src.imm_val);
        }
    } else if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x29);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported SUB operands");
    }
}

void Assembler::encode_xor(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x31);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported XOR operands");
    }
}

void Assembler::encode_and(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x21);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else if (dest.type == REG && src.type == IMM) {
        Assembler::emit_byte(0x81);
        Assembler::emit_byte(modrm_byte(3, 4, dest.reg_val));
        Assembler::emit_dword(src.imm_val);
    } else {
        throw std::runtime_error("Unsupported AND operands");
    }
}

void Assembler::encode_or(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x09);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported OR operands");
    }
}

void Assembler::encode_cmp(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        if (dest.reg_val == 0) {
            Assembler::emit_byte(0x3D);
            Assembler::emit_dword(src.imm_val);
        } else {
            Assembler::emit_byte(0x81);
            Assembler::emit_byte(modrm_byte(3, 7, dest.reg_val));
            Assembler::emit_dword(src.imm_val);
        }
    } else if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x39);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported CMP operands");
    }
}

void Assembler::encode_test(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x85);
        Assembler::emit_byte(modrm_byte(3, src.reg_val, dest.reg_val));
    } else {
        throw std::runtime_error("Unsupported TEST operands");
    }
}

void Assembler::encode_inc(const Operand& op) {
    if (op.type == REG) {
        Assembler::emit_byte(0x40 + op.reg_val);
    } else {
        throw std::runtime_error("Unsupported INC operand");
    }
}

void Assembler::encode_dec(const Operand& op) {
    if (op.type == REG) {
        Assembler::emit_byte(0x48 + op.reg_val);
    } else {
        throw std::runtime_error("Unsupported DEC operand");
    }
}

void Assembler::encode_push(const Operand& op) {
    if (op.type == REG) {
        Assembler::emit_byte(0x50 + op.reg_val);
    } else if (op.type == IMM) {
        if (op.imm_val >= -128 && op.imm_val <= 127) {
            Assembler::emit_byte(0x6A);
            Assembler::emit_byte(op.imm_val & 0xFF);
        } else {
            Assembler::emit_byte(0x68);
            Assembler::emit_dword(op.imm_val);
        }
    } else {
        throw std::runtime_error("Unsupported PUSH operand");
    }
}

void Assembler::encode_pop(const Operand& op) {
    if (op.type == REG) {
        Assembler::emit_byte(0x58 + op.reg_val);
    } else {
        throw std::runtime_error("Unsupported POP operand");
    }
}

void Assembler::encode_call(const Operand& op) {
    Assembler::emit_byte(0xE8);
    if (op.type == LABEL) {
        label_fixups[code.size()] = op.label_name;
        Assembler::emit_dword(0);
    } else if (op.type == IMM) {
        int32_t rel_offset = op.imm_val - (current_address + code.size() + 4);
        Assembler::emit_dword(rel_offset);
    } else {
        throw std::runtime_error("Unsupported CALL operand");
    }
}

void Assembler::encode_ret() {
    Assembler::emit_byte(0xC3);
}

void Assembler::encode_jmp(const Operand& op) {
    Assembler::emit_byte(0xE9);
    if (op.type == LABEL) {
        label_fixups[code.size()] = op.label_name;
        Assembler::emit_dword(0);
    } else {
        throw std::runtime_error("Unsupported JMP operand");
    }
}

void Assembler::encode_conditional_jump(uint8_t opcode, const Operand& op) {
    Assembler::emit_byte(0x0F);
    Assembler::emit_byte(opcode);
    if (op.type == LABEL) {
        label_fixups[code.size()] = op.label_name;
        Assembler::emit_dword(0);
    } else {
        throw std::runtime_error("Unsupported conditional jump operand");
    }
}

void Assembler::encode_int(const Operand& op) {
    if (op.type == IMM) {
        Assembler::emit_byte(0xCD);
        Assembler::emit_byte(op.imm_val & 0xFF);
    } else {
        throw std::runtime_error("Unsupported INT operand");
    }
}

void Assembler::encode_nop() {
    Assembler::emit_byte(0x90);
}

void Assembler::encode_lea(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == MEM_REG) {
        Assembler::emit_byte(0x8D);
        Assembler::emit_byte(modrm_byte(0, dest.reg_val, src.reg_val));
    } else if (dest.type == REG && src.type == MEM_REG_DISP) {
        Assembler::emit_byte(0x8D);
        Assembler::emit_byte(modrm_byte(2, dest.reg_val, src.reg_val));
        Assembler::emit_dword(src.disp);
    } else {
        throw std::runtime_error("Unsupported LEA operands");
    }
}

void Assembler::encode_imul(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == REG) {
        Assembler::emit_byte(0x0F);
        Assembler::emit_byte(0xAF);
        Assembler::emit_byte(modrm_byte(3, dest.reg_val, src.reg_val));
    } else {
        throw std::runtime_error("Unsupported IMUL operands");
    }
}

void Assembler::encode_idiv(const Operand& op) {
    if (op.type == REG) {
        Assembler::emit_byte(0xF7);
        Assembler::emit_byte(modrm_byte(3, 7, op.reg_val));
    } else {
        throw std::runtime_error("Unsupported IDIV operand");
    }
}

void Assembler::encode_shl(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        if (src.imm_val == 1) {
            Assembler::emit_byte(0xD1);
            Assembler::emit_byte(modrm_byte(3, 4, dest.reg_val));
        } else {
            Assembler::emit_byte(0xC1);
            Assembler::emit_byte(modrm_byte(3, 4, dest.reg_val));
            Assembler::emit_byte(src.imm_val & 0xFF);
        }
    } else {
        throw std::runtime_error("Unsupported SHL operands");
    }
}

void Assembler::encode_shr(const Operand& dest, const Operand& src) {
    if (dest.type == REG && src.type == IMM) {
        if (src.imm_val == 1) {
            Assembler::emit_byte(0xD1);
            Assembler::emit_byte(modrm_byte(3, 5, dest.reg_val));
        } else {
            Assembler::emit_byte(0xC1);
            Assembler::emit_byte(modrm_byte(3, 5, dest.reg_val));
            Assembler::emit_byte(src.imm_val & 0xFF);
        }
    } else {
        throw std::runtime_error("Unsupported SHR operands");
    }
}
