use crate::parser::{Directive, Instruction, Item, Operand};
use std::collections::HashMap;

/// A section of encoded bytes.
#[derive(Debug, Default, Clone)]
pub struct Section {
    pub name: String,
    pub data: Vec<u8>,
    pub globals: Vec<String>,
    /// Map from label name → byte offset within this section
    pub labels: HashMap<String, u64>,
    /// Relocations: (offset_in_section, label_name, is_relative)
    pub relocations: Vec<Relocation>,
}

#[derive(Debug, Clone)]
pub struct Relocation {
    pub offset: u64,
    pub symbol: String,
    pub addend: i64,
    pub is_relative: bool, // true = PC-relative (R_X86_64_PC32), false = absolute
}

pub struct EncoderOutput {
    pub sections: Vec<Section>,
}

pub fn encode(items: &[Item]) -> Result<EncoderOutput, String> {
    let mut sections: Vec<Section> = Vec::new();
    let mut current_section = Section {
        name: ".text".to_string(),
        ..Default::default()
    };

    // First pass: collect labels
    // Second pass: emit bytes (we do both in one pass with fixups for forward refs)
    let mut pending_globals: Vec<String> = Vec::new();
    let mut pending_label: Option<String> = None;

    for item in items {
        match item {
            Item::Directive(d) => match d {
                Directive::Section(name) => {
                    // Push current section, start new one
                    let old = std::mem::replace(
                        &mut current_section,
                        Section {
                            name: name.clone(),
                            ..Default::default()
                        },
                    );
                    if !old.data.is_empty() || !old.labels.is_empty() {
                        sections.push(old);
                    }
                }
                Directive::Global(sym) => {
                    pending_globals.push(sym.trim().to_string());
                }
                Directive::Byte(vals) => {
                    for &v in vals {
                        current_section.data.push(v as u8);
                    }
                }
                Directive::Word(vals) => {
                    for &v in vals {
                        current_section.data.extend_from_slice(&(v as u16).to_le_bytes());
                    }
                }
                Directive::Long(vals) => {
                    for &v in vals {
                        current_section.data.extend_from_slice(&(v as u32).to_le_bytes());
                    }
                }
                Directive::Quad(vals) => {
                    for &v in vals {
                        current_section.data.extend_from_slice(&v.to_le_bytes());
                    }
                }
                Directive::Ascii(s) => {
                    current_section.data.extend_from_slice(s.as_bytes());
                }
                Directive::Asciz(s) => {
                    current_section.data.extend_from_slice(s.as_bytes());
                    current_section.data.push(0);
                }
                Directive::Zero(n) => {
                    current_section.data.extend(std::iter::repeat(0u8).take(*n));
                }
            },
            Item::Instruction(instr) => {
                // Register any label
                if let Some(lbl) = &instr.label {
                    let offset = current_section.data.len() as u64;
                    current_section.labels.insert(lbl.clone(), offset);
                    // Attach pending globals
                    if pending_globals.contains(lbl) {
                        current_section.globals.push(lbl.clone());
                        pending_globals.retain(|g| g != lbl);
                    }
                }

                // Encode mnemonic if present
                if let Some(mnemonic) = &instr.mnemonic {
                    encode_instruction(mnemonic, &instr.operands, &mut current_section)?;
                }
            }
        }
    }

    // Flush any remaining globals (not yet matched to a label) to the current section
    for g in &pending_globals {
        current_section.globals.push(g.clone());
    }

    sections.push(current_section);
    Ok(EncoderOutput { sections })
}

// ---------------------------------------------------------------------------
// Instruction encoding
// ---------------------------------------------------------------------------

fn encode_instruction(
    mnemonic: &str,
    operands: &[Operand],
    sec: &mut Section,
) -> Result<(), String> {
    match mnemonic {
        // ── No-operand instructions ─────────────────────────────────────────
        "nop"   => sec.data.push(0x90),
        "ret"   => sec.data.push(0xC3),
        "retf"  => sec.data.push(0xCB),
        "leave" => sec.data.push(0xC9),
        "hlt"   => sec.data.push(0xF4),
        "cli"   => sec.data.push(0xFA),
        "sti"   => sec.data.push(0xFB),
        "cld"   => sec.data.push(0xFC),
        "std"   => sec.data.push(0xFD),
        "pushf" => sec.data.extend_from_slice(&[0x66, 0x9C]),
        "pushfq"=> sec.data.push(0x9C),
        "popf"  => sec.data.extend_from_slice(&[0x66, 0x9D]),
        "popfq" => sec.data.push(0x9D),
        "syscall" => sec.data.extend_from_slice(&[0x0F, 0x05]),
        "sysret"  => sec.data.extend_from_slice(&[0x0F, 0x07]),
        "cpuid"   => sec.data.extend_from_slice(&[0x0F, 0xA2]),
        "rdtsc"   => sec.data.extend_from_slice(&[0x0F, 0x31]),
        "pause"   => sec.data.extend_from_slice(&[0xF3, 0x90]),
        "ud2"     => sec.data.extend_from_slice(&[0x0F, 0x0B]),
        "endbr64" => sec.data.extend_from_slice(&[0xF3, 0x0F, 0x1E, 0xFA]),

        // ── Single-operand (64-bit reg) ─────────────────────────────────────
        "push" => encode_push(operands, sec)?,
        "pop"  => encode_pop(operands, sec)?,
        "inc"  => encode_unary_rm(0xFF, 0, operands, sec)?,
        "dec"  => encode_unary_rm(0xFF, 1, operands, sec)?,
        "neg"  => encode_unary_rm(0xF7, 3, operands, sec)?,
        "not"  => encode_unary_rm(0xF7, 2, operands, sec)?,
        "idiv" => encode_unary_rm(0xF7, 7, operands, sec)?,
        "div"  => encode_unary_rm(0xF7, 6, operands, sec)?,
        "imul1"=> encode_unary_rm(0xF7, 5, operands, sec)?,
        "mul"  => encode_unary_rm(0xF7, 4, operands, sec)?,

        // ── Two-operand (MOV, ADD, SUB, etc.) ──────────────────────────────
        "mov"  => encode_mov(operands, sec)?,
        "movsx"  => encode_movsx(operands, sec, false)?,
        "movsxd" => encode_movsx(operands, sec, true)?,
        "movzx"  => encode_movzx(operands, sec)?,
        "lea"  => encode_lea(operands, sec)?,
        "add"  => encode_alu(0x01, 0x03, 0x81, 0, operands, sec)?,
        "sub"  => encode_alu(0x29, 0x2B, 0x81, 5, operands, sec)?,
        "and"  => encode_alu(0x21, 0x23, 0x81, 4, operands, sec)?,
        "or"   => encode_alu(0x09, 0x0B, 0x81, 1, operands, sec)?,
        "xor"  => encode_alu(0x31, 0x33, 0x81, 6, operands, sec)?,
        "cmp"  => encode_alu(0x39, 0x3B, 0x81, 7, operands, sec)?,
        "test" => encode_test(operands, sec)?,
        "imul" => encode_imul(operands, sec)?,
        "xchg" => encode_xchg(operands, sec)?,

        // ── Shifts ──────────────────────────────────────────────────────────
        "shl" | "sal" => encode_shift(4, operands, sec)?,
        "shr"         => encode_shift(5, operands, sec)?,
        "sar"         => encode_shift(7, operands, sec)?,
        "rol"         => encode_shift(0, operands, sec)?,
        "ror"         => encode_shift(1, operands, sec)?,

        // ── Jumps and calls ─────────────────────────────────────────────────
        "jmp"  => encode_jmp(0xEB, 0xE9, operands, sec)?,
        "je"  | "jz"   => encode_jcc(0x74, 0x84, operands, sec)?,
        "jne" | "jnz"  => encode_jcc(0x75, 0x85, operands, sec)?,
        "jl"  | "jnge" => encode_jcc(0x7C, 0x8C, operands, sec)?,
        "jge" | "jnl"  => encode_jcc(0x7D, 0x8D, operands, sec)?,
        "jle" | "jng"  => encode_jcc(0x7E, 0x8E, operands, sec)?,
        "jg"  | "jnle" => encode_jcc(0x7F, 0x8F, operands, sec)?,
        "jb"  | "jc" | "jnae" => encode_jcc(0x72, 0x82, operands, sec)?,
        "jae" | "jnb" | "jnc" => encode_jcc(0x73, 0x83, operands, sec)?,
        "jbe" | "jna"  => encode_jcc(0x76, 0x86, operands, sec)?,
        "ja"  | "jnbe" => encode_jcc(0x77, 0x87, operands, sec)?,
        "js"           => encode_jcc(0x78, 0x88, operands, sec)?,
        "jns"          => encode_jcc(0x79, 0x89, operands, sec)?,
        "jp"  | "jpe"  => encode_jcc(0x7A, 0x8A, operands, sec)?,
        "jnp" | "jpo"  => encode_jcc(0x7B, 0x8B, operands, sec)?,
        "jo"           => encode_jcc(0x70, 0x80, operands, sec)?,
        "jno"          => encode_jcc(0x71, 0x81, operands, sec)?,
        "call" => encode_call(operands, sec)?,

        // ── String ops ──────────────────────────────────────────────────────
        "rep"   => { sec.data.push(0xF3); }
        "repne" | "repnz" => { sec.data.push(0xF2); }
        "movsb" => sec.data.push(0xA4),
        "movsw" => sec.data.extend_from_slice(&[0x66, 0xA5]),
        "movsd" => sec.data.push(0xA5),
        "movsq" => sec.data.extend_from_slice(&[0x48, 0xA5]),
        "cmpsb" => sec.data.push(0xA6),
        "scasb" => sec.data.push(0xAE),
        "stosb" => sec.data.push(0xAA),
        "stosd" => sec.data.push(0xAB),
        "stosq" => sec.data.extend_from_slice(&[0x48, 0xAB]),
        "lodsb" => sec.data.push(0xAC),
        "lodsq" => sec.data.extend_from_slice(&[0x48, 0xAD]),

        // ── Set byte on condition ────────────────────────────────────────────
        "sete"  | "setz"   => encode_setcc(0x94, operands, sec)?,
        "setne" | "setnz"  => encode_setcc(0x95, operands, sec)?,
        "setl"  | "setnge" => encode_setcc(0x9C, operands, sec)?,
        "setge" | "setnl"  => encode_setcc(0x9D, operands, sec)?,
        "setle" | "setng"  => encode_setcc(0x9E, operands, sec)?,
        "setg"  | "setnle" => encode_setcc(0x9F, operands, sec)?,
        "setb"  | "setc"   => encode_setcc(0x92, operands, sec)?,
        "setae" | "setnb"  => encode_setcc(0x93, operands, sec)?,
        "sets"  => encode_setcc(0x98, operands, sec)?,
        "setns" => encode_setcc(0x99, operands, sec)?,

        // ── CMOV ────────────────────────────────────────────────────────────
        "cmove"  | "cmovz"   => encode_cmov(0x44, operands, sec)?,
        "cmovne" | "cmovnz"  => encode_cmov(0x45, operands, sec)?,
        "cmovl"  | "cmovnge" => encode_cmov(0x4C, operands, sec)?,
        "cmovge" | "cmovnl"  => encode_cmov(0x4D, operands, sec)?,
        "cmovle" | "cmovng"  => encode_cmov(0x4E, operands, sec)?,
        "cmovg"  | "cmovnle" => encode_cmov(0x4F, operands, sec)?,

        // ── Misc ─────────────────────────────────────────────────────────────
        "int" => {
            let imm = require_imm(operands, 0)?;
            sec.data.push(0xCD);
            sec.data.push(imm as u8);
        }
        "int3" => sec.data.push(0xCC),
        "nopw" => sec.data.extend_from_slice(&[0x66, 0x90]),
        "xlatb" => sec.data.push(0xD7),

        unknown => {
            return Err(format!("unknown mnemonic: '{}'", unknown));
        }
    }
    Ok(())
}

// ---------------------------------------------------------------------------
// Register encoding helpers
// ---------------------------------------------------------------------------

/// Returns (rex_b_needed, reg_bits_3) for a 64-bit register name.
fn reg_info(name: &str) -> Option<(bool, u8)> {
    match name {
        "rax" | "eax" | "ax" | "al" => Some((false, 0)),
        "rcx" | "ecx" | "cx" | "cl" => Some((false, 1)),
        "rdx" | "edx" | "dx" | "dl" => Some((false, 2)),
        "rbx" | "ebx" | "bx" | "bl" => Some((false, 3)),
        "rsp" | "esp" | "sp" | "ah" | "spl" => Some((false, 4)),
        "rbp" | "ebp" | "bp" | "ch" | "bpl" => Some((false, 5)),
        "rsi" | "esi" | "si" | "dh" | "sil" => Some((false, 6)),
        "rdi" | "edi" | "di" | "bh" | "dil" => Some((false, 7)),
        "r8"  | "r8d"  | "r8w"  | "r8b"  => Some((true,  0)),
        "r9"  | "r9d"  | "r9w"  | "r9b"  => Some((true,  1)),
        "r10" | "r10d" | "r10w" | "r10b" => Some((true,  2)),
        "r11" | "r11d" | "r11w" | "r11b" => Some((true,  3)),
        "r12" | "r12d" | "r12w" | "r12b" => Some((true,  4)),
        "r13" | "r13d" | "r13w" | "r13b" => Some((true,  5)),
        "r14" | "r14d" | "r14w" | "r14b" => Some((true,  6)),
        "r15" | "r15d" | "r15w" | "r15b" => Some((true,  7)),
        _ => None,
    }
}

fn is_64bit_reg(name: &str) -> bool {
    name.starts_with('r') && !name.starts_with("r8b")
        && !name.starts_with("r9b")
        && !name.ends_with('d')
        && !name.ends_with('w')
        && !name.ends_with('b')
        || name.len() == 2 && name.starts_with('r')
        || matches!(name, "rax"|"rbx"|"rcx"|"rdx"|"rsi"|"rdi"|"rsp"|"rbp"|"rip"
            |"r8"|"r9"|"r10"|"r11"|"r12"|"r13"|"r14"|"r15")
}

/// Build a REX prefix byte: W=1 for 64-bit, R=rex_r, X=0, B=rex_b.
fn rex(w: bool, r_ext: bool, x_ext: bool, b_ext: bool) -> u8 {
    0x40
        | if w     { 0x08 } else { 0 }
        | if r_ext { 0x04 } else { 0 }
        | if x_ext { 0x02 } else { 0 }
        | if b_ext { 0x01 } else { 0 }
}

/// Build ModRM byte: mod=3 (register), reg field = `reg`, rm field = `rm`.
fn modrm_rr(reg: u8, rm: u8) -> u8 {
    0xC0 | ((reg & 7) << 3) | (rm & 7)
}

fn modrm_mod_reg_rm(md: u8, reg: u8, rm: u8) -> u8 {
    ((md & 3) << 6) | ((reg & 7) << 3) | (rm & 7)
}

fn require_reg(operands: &[Operand], idx: usize) -> Result<&str, String> {
    match operands.get(idx) {
        Some(Operand::Register(r)) => Ok(r.as_str()),
        other => Err(format!("expected register at operand {}, got {:?}", idx, other)),
    }
}

fn require_imm(operands: &[Operand], idx: usize) -> Result<i64, String> {
    match operands.get(idx) {
        Some(Operand::Immediate(v)) => Ok(*v),
        other => Err(format!("expected immediate at operand {}, got {:?}", idx, other)),
    }
}

// ---------------------------------------------------------------------------
// Specific encoders
// ---------------------------------------------------------------------------

fn encode_push(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    match ops.get(0) {
        Some(Operand::Register(r)) => {
            let (ext, bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
            if ext { sec.data.push(rex(false, false, false, true)); }
            sec.data.push(0x50 | bits);
            Ok(())
        }
        Some(Operand::Immediate(v)) => {
            let v = *v;
            if v >= -128 && v <= 127 {
                sec.data.push(0x6A);
                sec.data.push(v as i8 as u8);
            } else {
                sec.data.push(0x68);
                sec.data.extend_from_slice(&(v as i32).to_le_bytes());
            }
            Ok(())
        }
        other => Err(format!("push: unsupported operand {:?}", other)),
    }
}

fn encode_pop(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    let r = require_reg(ops, 0)?;
    let (ext, bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
    if ext { sec.data.push(rex(false, false, false, true)); }
    sec.data.push(0x58 | bits);
    Ok(())
}

fn encode_unary_rm(opcode: u8, reg_field: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    let r = require_reg(ops, 0)?;
    let (ext, bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
    let w = is_64bit_reg(r);
    sec.data.push(rex(w, false, false, ext));
    sec.data.push(opcode);
    sec.data.push(modrm_rr(reg_field, bits));
    Ok(())
}

fn encode_mov(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 {
        return Err("mov requires 2 operands".into());
    }
    match (&ops[0], &ops[1]) {
        // mov reg, imm
        (Operand::Register(dst), Operand::Immediate(imm)) => {
            let (ext, bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let w = is_64bit_reg(dst);
            if w {
                let imm_val = *imm;
                if imm_val >= i32::MIN as i64 && imm_val <= i32::MAX as i64 {
                    // MOV r/m64, imm32 (sign-extended)
                    sec.data.push(rex(true, false, false, ext));
                    sec.data.push(0xC7);
                    sec.data.push(modrm_rr(0, bits));
                    sec.data.extend_from_slice(&(imm_val as i32).to_le_bytes());
                } else {
                    // MOV r64, imm64
                    sec.data.push(rex(true, false, false, ext));
                    sec.data.push(0xB8 | bits);
                    sec.data.extend_from_slice(&imm_val.to_le_bytes());
                }
            } else {
                if ext { sec.data.push(rex(false, false, false, true)); }
                sec.data.push(0xB8 | bits);
                sec.data.extend_from_slice(&(*imm as i32).to_le_bytes());
            }
            Ok(())
        }
        // mov reg, reg
        (Operand::Register(dst), Operand::Register(src)) => {
            let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
            let w = is_64bit_reg(dst) || is_64bit_reg(src);
            sec.data.push(rex(w, src_ext, false, dst_ext));
            sec.data.push(if w { 0x89 } else { 0x88 });
            sec.data.push(modrm_rr(src_bits, dst_bits));
            Ok(())
        }
        // mov reg, mem
        (Operand::Register(dst), Operand::Memory { displacement, base, index, scale }) => {
            let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let w = is_64bit_reg(dst);
            encode_mem_operand(
                if w { 0x8B } else { 0x8A },
                dst_bits, dst_ext, *displacement,
                base.as_deref(), index.as_deref(), *scale, w, sec,
            )
        }
        // mov mem, reg
        (Operand::Memory { displacement, base, index, scale }, Operand::Register(src)) => {
            let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
            let w = is_64bit_reg(src);
            encode_mem_operand(
                if w { 0x89 } else { 0x88 },
                src_bits, src_ext, *displacement,
                base.as_deref(), index.as_deref(), *scale, w, sec,
            )
        }
        // mov mem, imm  →  MOV r/m64, imm32
        (Operand::Memory { displacement, base, index, scale }, Operand::Immediate(imm)) => {
            // use 64-bit form
            encode_mem_operand_imm(
                0xC7, 0, *displacement,
                base.as_deref(), index.as_deref(), *scale, true, *imm as i32, sec,
            )
        }
        (a, b) => Err(format!("mov: unsupported operands {:?}, {:?}", a, b)),
    }
}

fn encode_movsx(ops: &[Operand], sec: &mut Section, is_64: bool) -> Result<(), String> {
    if ops.len() < 2 { return Err("movsx/movsxd requires 2 operands".into()); }
    let dst = require_reg(ops, 0)?;
    let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
    match &ops[1] {
        Operand::Register(src) => {
            let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
            if is_64 {
                // MOVSXD r64, r/m32
                sec.data.push(rex(true, dst_ext, false, src_ext));
                sec.data.push(0x63);
                sec.data.push(modrm_rr(dst_bits, src_bits));
            } else {
                // MOVSX r64, r/m8  (0x0F 0xBE)
                sec.data.push(rex(true, dst_ext, false, src_ext));
                sec.data.extend_from_slice(&[0x0F, 0xBE]);
                sec.data.push(modrm_rr(dst_bits, src_bits));
            }
            Ok(())
        }
        other => Err(format!("movsx: unsupported src operand {:?}", other)),
    }
}

fn encode_movzx(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 { return Err("movzx requires 2 operands".into()); }
    let dst = require_reg(ops, 0)?;
    let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
    let src = require_reg(ops, 1)?;
    let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
    sec.data.push(rex(true, dst_ext, false, src_ext));
    sec.data.extend_from_slice(&[0x0F, 0xB6]);
    sec.data.push(modrm_rr(dst_bits, src_bits));
    Ok(())
}

fn encode_lea(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 { return Err("lea requires 2 operands".into()); }
    let dst = require_reg(ops, 0)?;
    let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
    match &ops[1] {
        Operand::Memory { displacement, base, index, scale } => {
            encode_mem_operand(0x8D, dst_bits, dst_ext, *displacement,
                base.as_deref(), index.as_deref(), *scale, true, sec)
        }
        Operand::Label(name) => {
            // LEA dst, [rip + label]  — RIP-relative
            sec.data.push(rex(true, dst_ext, false, false));
            sec.data.push(0x8D);
            sec.data.push(modrm_mod_reg_rm(0, dst_bits, 5)); // mod=00, rm=101 = RIP-relative
            let reloc_offset = sec.data.len() as u64;
            sec.relocations.push(Relocation {
                offset: reloc_offset,
                symbol: name.clone(),
                addend: -4,
                is_relative: true,
            });
            sec.data.extend_from_slice(&0i32.to_le_bytes());
            Ok(())
        }
        other => Err(format!("lea: unsupported src {:?}", other)),
    }
}

/// Generic ALU: opcode_rm_r, opcode_r_rm, opcode_imm, reg_field_for_imm
fn encode_alu(
    op_rm_r: u8, op_r_rm: u8, op_imm: u8, reg_field: u8,
    ops: &[Operand], sec: &mut Section,
) -> Result<(), String> {
    if ops.len() < 2 { return Err("ALU op requires 2 operands".into()); }
    match (&ops[0], &ops[1]) {
        (Operand::Register(dst), Operand::Register(src)) => {
            let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
            let w = is_64bit_reg(dst) || is_64bit_reg(src);
            sec.data.push(rex(w, src_ext, false, dst_ext));
            sec.data.push(op_rm_r);
            sec.data.push(modrm_rr(src_bits, dst_bits));
            Ok(())
        }
        (Operand::Register(dst), Operand::Immediate(imm)) => {
            let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let w = is_64bit_reg(dst);
            let v = *imm;
            if v >= -128 && v <= 127 {
                sec.data.push(rex(w, false, false, dst_ext));
                sec.data.push(0x83);
                sec.data.push(modrm_rr(reg_field, dst_bits));
                sec.data.push(v as i8 as u8);
            } else {
                sec.data.push(rex(w, false, false, dst_ext));
                sec.data.push(op_imm);
                sec.data.push(modrm_rr(reg_field, dst_bits));
                sec.data.extend_from_slice(&(v as i32).to_le_bytes());
            }
            Ok(())
        }
        (Operand::Register(dst), Operand::Memory { displacement, base, index, scale }) => {
            let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
            let w = is_64bit_reg(dst);
            encode_mem_operand(op_r_rm, dst_bits, dst_ext, *displacement,
                base.as_deref(), index.as_deref(), *scale, w, sec)
        }
        (a, b) => Err(format!("ALU: unsupported operands {:?}, {:?}", a, b)),
    }
}

fn encode_test(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 { return Err("test requires 2 operands".into()); }
    match (&ops[0], &ops[1]) {
        (Operand::Register(a), Operand::Register(b)) => {
            let (a_ext, a_bits) = reg_info(a).ok_or_else(|| format!("unknown register: {}", a))?;
            let (b_ext, b_bits) = reg_info(b).ok_or_else(|| format!("unknown register: {}", b))?;
            let w = is_64bit_reg(a) || is_64bit_reg(b);
            sec.data.push(rex(w, b_ext, false, a_ext));
            sec.data.push(if w { 0x85 } else { 0x84 });
            sec.data.push(modrm_rr(b_bits, a_bits));
            Ok(())
        }
        (Operand::Register(r), Operand::Immediate(imm)) => {
            let (r_ext, r_bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
            let w = is_64bit_reg(r);
            sec.data.push(rex(w, false, false, r_ext));
            sec.data.push(if w { 0xF7 } else { 0xF6 });
            sec.data.push(modrm_rr(0, r_bits));
            sec.data.extend_from_slice(&(*imm as i32).to_le_bytes());
            Ok(())
        }
        (a, b) => Err(format!("test: unsupported {:?} {:?}", a, b)),
    }
}

fn encode_imul(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() == 1 {
        return encode_unary_rm(0xF7, 5, ops, sec);
    }
    if ops.len() >= 2 {
        let dst = require_reg(ops, 0)?;
        let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
        match &ops[1] {
            Operand::Register(src) => {
                let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
                let w = is_64bit_reg(dst);
                if ops.len() == 3 {
                    // IMUL r64, r/m64, imm
                    let imm = require_imm(ops, 2)?;
                    sec.data.push(rex(w, dst_ext, false, src_ext));
                    if imm >= -128 && imm <= 127 {
                        sec.data.push(0x6B);
                        sec.data.push(modrm_rr(dst_bits, src_bits));
                        sec.data.push(imm as i8 as u8);
                    } else {
                        sec.data.push(0x69);
                        sec.data.push(modrm_rr(dst_bits, src_bits));
                        sec.data.extend_from_slice(&(imm as i32).to_le_bytes());
                    }
                } else {
                    // IMUL r64, r/m64
                    sec.data.push(rex(w, dst_ext, false, src_ext));
                    sec.data.extend_from_slice(&[0x0F, 0xAF]);
                    sec.data.push(modrm_rr(dst_bits, src_bits));
                }
                Ok(())
            }
            other => Err(format!("imul: unsupported src {:?}", other)),
        }
    } else {
        Err("imul: requires at least 1 operand".into())
    }
}

fn encode_xchg(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 { return Err("xchg requires 2 operands".into()); }
    let a = require_reg(ops, 0)?;
    let b = require_reg(ops, 1)?;
    let (a_ext, a_bits) = reg_info(a).ok_or_else(|| format!("unknown register: {}", a))?;
    let (b_ext, b_bits) = reg_info(b).ok_or_else(|| format!("unknown register: {}", b))?;
    let w = is_64bit_reg(a) || is_64bit_reg(b);
    // Short form: XCHG rAX, r64
    if a_bits == 0 && !a_ext && w {
        if b_ext { sec.data.push(rex(true, false, false, true)); } else { sec.data.push(rex(true, false, false, false)); }
        sec.data.push(0x90 | b_bits);
    } else {
        sec.data.push(rex(w, a_ext, false, b_ext));
        sec.data.push(if w { 0x87 } else { 0x86 });
        sec.data.push(modrm_rr(a_bits, b_bits));
    }
    Ok(())
}

fn encode_shift(reg_field: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.is_empty() { return Err("shift requires operands".into()); }
    let r = require_reg(ops, 0)?;
    let (r_ext, r_bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
    let w = is_64bit_reg(r);
    sec.data.push(rex(w, false, false, r_ext));
    if ops.len() >= 2 {
        match &ops[1] {
            Operand::Immediate(1) => {
                sec.data.push(if w { 0xD1 } else { 0xD0 });
                sec.data.push(modrm_rr(reg_field, r_bits));
            }
            Operand::Immediate(n) => {
                sec.data.push(if w { 0xC1 } else { 0xC0 });
                sec.data.push(modrm_rr(reg_field, r_bits));
                sec.data.push(*n as u8);
            }
            Operand::Register(cl) if cl == "cl" => {
                sec.data.push(if w { 0xD3 } else { 0xD2 });
                sec.data.push(modrm_rr(reg_field, r_bits));
            }
            other => return Err(format!("shift: invalid count operand {:?}", other)),
        }
    } else {
        // Default shift by 1
        sec.data.push(if w { 0xD1 } else { 0xD0 });
        sec.data.push(modrm_rr(reg_field, r_bits));
    }
    Ok(())
}

fn encode_jmp(short_op: u8, near_op: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    match ops.get(0) {
        Some(Operand::Label(name)) => {
            // Emit as near jump with relocation; linker resolves
            sec.data.push(near_op);
            let rel_offset = sec.data.len() as u64;
            sec.relocations.push(Relocation {
                offset: rel_offset,
                symbol: name.clone(),
                addend: -4,
                is_relative: true,
            });
            sec.data.extend_from_slice(&0i32.to_le_bytes());
            Ok(())
        }
        Some(Operand::Register(r)) => {
            let (r_ext, r_bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
            if r_ext { sec.data.push(rex(false, false, false, true)); }
            sec.data.push(0xFF);
            sec.data.push(modrm_rr(4, r_bits));
            Ok(())
        }
        other => Err(format!("jmp: unsupported operand {:?}", other)),
    }
}

fn encode_jcc(short_op: u8, near_op: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    match ops.get(0) {
        Some(Operand::Label(name)) => {
            sec.data.extend_from_slice(&[0x0F, near_op]);
            let rel_offset = sec.data.len() as u64;
            sec.relocations.push(Relocation {
                offset: rel_offset,
                symbol: name.clone(),
                addend: -4,
                is_relative: true,
            });
            sec.data.extend_from_slice(&0i32.to_le_bytes());
            Ok(())
        }
        other => Err(format!("jcc: unsupported operand {:?}", other)),
    }
}

fn encode_call(ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    match ops.get(0) {
        Some(Operand::Label(name)) => {
            sec.data.push(0xE8);
            let rel_offset = sec.data.len() as u64;
            sec.relocations.push(Relocation {
                offset: rel_offset,
                symbol: name.clone(),
                addend: -4,
                is_relative: true,
            });
            sec.data.extend_from_slice(&0i32.to_le_bytes());
            Ok(())
        }
        Some(Operand::Register(r)) => {
            let (r_ext, r_bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
            if r_ext { sec.data.push(rex(false, false, false, true)); }
            sec.data.push(0xFF);
            sec.data.push(modrm_rr(2, r_bits));
            Ok(())
        }
        other => Err(format!("call: unsupported operand {:?}", other)),
    }
}

fn encode_setcc(op2: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    let r = require_reg(ops, 0)?;
    let (r_ext, r_bits) = reg_info(r).ok_or_else(|| format!("unknown register: {}", r))?;
    if r_ext { sec.data.push(rex(false, false, false, true)); }
    sec.data.extend_from_slice(&[0x0F, op2]);
    sec.data.push(modrm_rr(0, r_bits));
    Ok(())
}

fn encode_cmov(op2: u8, ops: &[Operand], sec: &mut Section) -> Result<(), String> {
    if ops.len() < 2 { return Err("cmov requires 2 operands".into()); }
    let dst = require_reg(ops, 0)?;
    let src = require_reg(ops, 1)?;
    let (dst_ext, dst_bits) = reg_info(dst).ok_or_else(|| format!("unknown register: {}", dst))?;
    let (src_ext, src_bits) = reg_info(src).ok_or_else(|| format!("unknown register: {}", src))?;
    let w = is_64bit_reg(dst);
    sec.data.push(rex(w, dst_ext, false, src_ext));
    sec.data.extend_from_slice(&[0x0F, op2]);
    sec.data.push(modrm_rr(dst_bits, src_bits));
    Ok(())
}

/// Encode an instruction that uses a ModRM+SIB memory addressing
fn encode_mem_operand(
    opcode: u8,
    reg_bits: u8,
    reg_ext: bool,
    disp: i64,
    base: Option<&str>,
    index: Option<&str>,
    scale: u8,
    w: bool,
    sec: &mut Section,
) -> Result<(), String> {
    let (base_ext, base_bits) = if let Some(b) = base {
        reg_info(b).ok_or_else(|| format!("unknown base register: {}", b))?
    } else {
        (false, 5u8) // RIP-relative or disp32
    };

    let (idx_ext, idx_bits) = if let Some(i) = index {
        reg_info(i).ok_or_else(|| format!("unknown index register: {}", i))?
    } else {
        (false, 4u8) // no index
    };

    let need_sib = index.is_some() || base_bits == 4; // rsp/r12 base → need SIB

    // REX prefix
    let rex_byte = rex(w, reg_ext, idx_ext, base_ext);
    if rex_byte != 0x40 {
        sec.data.push(rex_byte);
    }
    sec.data.push(opcode);

    let (mod_bits, extra_bytes): (u8, usize) = if base.is_none() {
        (0, 4)
    } else if disp == 0 && base_bits != 5 {
        (0, 0)
    } else if disp >= -128 && disp <= 127 {
        (1, 1)
    } else {
        (2, 4)
    };

    if need_sib {
        sec.data.push(modrm_mod_reg_rm(mod_bits, reg_bits, 4)); // rm=100 → SIB follows
        let scale_bits: u8 = match scale { 1 => 0, 2 => 1, 4 => 2, 8 => 3, _ => 0 };
        sec.data.push((scale_bits << 6) | ((idx_bits & 7) << 3) | (base_bits & 7));
    } else {
        sec.data.push(modrm_mod_reg_rm(mod_bits, reg_bits, base_bits & 7));
    }

    match extra_bytes {
        1 => sec.data.push(disp as i8 as u8),
        4 => sec.data.extend_from_slice(&(disp as i32).to_le_bytes()),
        _ => {}
    }
    Ok(())
}

fn encode_mem_operand_imm(
    opcode: u8,
    reg_field: u8,
    disp: i64,
    base: Option<&str>,
    index: Option<&str>,
    scale: u8,
    w: bool,
    imm: i32,
    sec: &mut Section,
) -> Result<(), String> {
    encode_mem_operand(opcode, reg_field, false, disp, base, index, scale, w, sec)?;
    sec.data.extend_from_slice(&imm.to_le_bytes());
    Ok(())
}
