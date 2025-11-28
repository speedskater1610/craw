use std::collections::HashMap;
use std::fs::File;
use std::io::Write;
use std::os::raw::{c_char, c_int};
use std::ffi::CStr;

// Register encodings for IA-32
#[derive(Debug, Clone, Copy, PartialEq)]
enum Register {
    AL, AH, AX, EAX,
    CL, CH, CX, ECX,
    DL, DH, DX, EDX,
    BL, BH, BX, EBX,
    SP, ESP, BP, EBP,
    SI, ESI, DI, EDI,
    CS, DS, ES, FS, GS, SS,
}

impl Register {
    fn encoding(&self) -> u8 {
        match self {
            Register::AL | Register::AX | Register::EAX => 0,
            Register::CL | Register::CX | Register::ECX => 1,
            Register::DL | Register::DX | Register::EDX => 2,
            Register::BL | Register::BX | Register::EBX => 3,
            Register::AH => 4,
            Register::CH => 5,
            Register::DH => 6,
            Register::BH => 7,
            Register::SP | Register::ESP => 4,
            Register::BP | Register::EBP => 5,
            Register::SI | Register::ESI => 6,
            Register::DI | Register::EDI => 7,
            _ => 0,
        }
    }
}

#[derive(Debug, Clone)]
enum OpSize {
    Byte = 1,
    Word = 2,
    Dword = 4,
    Unknown = 0,
}

#[derive(Debug, Clone)]
struct Memory {
    size: OpSize,
    base: Option<u8>,
    index: Option<u8>,
    scale: u8,
    disp: i32,
}

#[derive(Debug, Clone)]
enum Operand {
    Register(u8),
    Immediate(i64),
    Memory(Memory),
}

#[derive(Debug)]
struct Instruction {
    mnemonic: String,
    operands: Vec<Operand>,
}

#[derive(Debug)]
enum Statement {
    Label(String),
    Instruction(Instruction),
    Directive(Directive),
}

#[derive(Debug)]
enum Directive {
    Section(String),
    Global(String),
    Extern(String),
    DB(Vec<i64>),
    DW(Vec<i64>),
    DD(Vec<i64>),
    ResB(usize),
    ResW(usize),
    ResD(usize),
    Align(usize),
}

// ELF structures
const EI_NIDENT: usize = 16;
const ET_REL: u16 = 1;
const EM_386: u16 = 3;
const SHT_NULL: u32 = 0;
const SHT_PROGBITS: u32 = 1;
const SHT_SYMTAB: u32 = 2;
const SHT_STRTAB: u32 = 3;
const SHT_NOBITS: u32 = 8;
const SHF_WRITE: u32 = 0x1;
const SHF_ALLOC: u32 = 0x2;
const SHF_EXECINSTR: u32 = 0x4;

#[repr(C)]
struct ElfHeader {
    e_ident: [u8; EI_NIDENT],
    e_type: u16,
    e_machine: u16,
    e_version: u32,
    e_entry: u32,
    e_phoff: u32,
    e_shoff: u32,
    e_flags: u32,
    e_ehsize: u16,
    e_phentsize: u16,
    e_phnum: u16,
    e_shentsize: u16,
    e_shnum: u16,
    e_shstrndx: u16,
}

#[repr(C)]
struct SectionHeader {
    sh_name: u32,
    sh_type: u32,
    sh_flags: u32,
    sh_addr: u32,
    sh_offset: u32,
    sh_size: u32,
    sh_link: u32,
    sh_info: u32,
    sh_addralign: u32,
    sh_entsize: u32,
}

#[repr(C)]
struct SymbolEntry {
    st_name: u32,
    st_value: u32,
    st_size: u32,
    st_info: u8,
    st_other: u8,
    st_shndx: u16,
}

struct Assembler {
    sections: HashMap<String, Section>,
    current_section: String,
    symbols: HashMap<String, Symbol>,
    global_symbols: Vec<String>,
}

struct Section {
    name: String,
    data: Vec<u8>,
    relocations: Vec<Relocation>,
    flags: u32,
    sh_type: u32,
}

struct Symbol {
    name: String,
    section: String,
    offset: usize,
    global: bool,
}

struct Relocation {
    offset: usize,
    symbol: String,
    rtype: u32,
}

impl Assembler {
    fn new() -> Self {
        let mut sections = HashMap::new();
        sections.insert(".text".to_string(), Section {
            name: ".text".to_string(),
            data: Vec::new(),
            relocations: Vec::new(),
            flags: SHF_ALLOC | SHF_EXECINSTR,
            sh_type: SHT_PROGBITS,
        });
        sections.insert(".data".to_string(), Section {
            name: ".data".to_string(),
            data: Vec::new(),
            relocations: Vec::new(),
            flags: SHF_ALLOC | SHF_WRITE,
            sh_type: SHT_PROGBITS,
        });
        sections.insert(".bss".to_string(), Section {
            name: ".bss".to_string(),
            data: Vec::new(),
            relocations: Vec::new(),
            flags: SHF_ALLOC | SHF_WRITE,
            sh_type: SHT_NOBITS,
        });
        
        Assembler {
            sections,
            current_section: ".text".to_string(),
            symbols: HashMap::new(),
            global_symbols: Vec::new(),
        }
    }

    fn process_statement(&mut self, stmt: Statement) {
        match stmt {
            Statement::Label(name) => {
                let offset = self.sections.get(&self.current_section)
                    .map(|s| s.data.len())
                    .unwrap_or(0);
                self.symbols.insert(name.clone(), Symbol {
                    name: name.clone(),
                    section: self.current_section.clone(),
                    offset,
                    global: self.global_symbols.contains(&name),
                });
            }
            Statement::Instruction(instr) => {
                self.encode_instruction(instr);
            }
            Statement::Directive(dir) => {
                self.process_directive(dir);
            }
        }
    }

    fn process_directive(&mut self, dir: Directive) {
        match dir {
            Directive::Section(name) => {
                self.current_section = name;
            }
            Directive::Global(name) => {
                self.global_symbols.push(name);
            }
            Directive::DB(values) => {
                if let Some(section) = self.sections.get_mut(&self.current_section) {
                    for val in values {
                        section.data.push(val as u8);
                    }
                }
            }
            Directive::DW(values) => {
                if let Some(section) = self.sections.get_mut(&self.current_section) {
                    for val in values {
                        section.data.extend_from_slice(&(val as u16).to_le_bytes());
                    }
                }
            }
            Directive::DD(values) => {
                if let Some(section) = self.sections.get_mut(&self.current_section) {
                    for val in values {
                        section.data.extend_from_slice(&(val as u32).to_le_bytes());
                    }
                }
            }
            Directive::ResB(count) => {
                if let Some(section) = self.sections.get_mut(&self.current_section) {
                    section.data.resize(section.data.len() + count, 0);
                }
            }
            Directive::Align(boundary) => {
                if let Some(section) = self.sections.get_mut(&self.current_section) {
                    let len = section.data.len();
                    let aligned = (len + boundary - 1) & !(boundary - 1);
                    section.data.resize(aligned, 0);
                }
            }
            _ => {}
        }
    }

    fn encode_instruction(&mut self, instr: Instruction) {
        let bytes = match instr.mnemonic.to_lowercase().as_str() {
            "mov" => self.encode_mov(&instr.operands),
            "add" => self.encode_alu(0, &instr.operands),
            "sub" => self.encode_alu(5, &instr.operands),
            "xor" => self.encode_alu(6, &instr.operands),
            "cmp" => self.encode_alu(7, &instr.operands),
            "push" => self.encode_push(&instr.operands),
            "pop" => self.encode_pop(&instr.operands),
            "call" => self.encode_call(&instr.operands),
            "ret" => vec![0xC3],
            "nop" => vec![0x90],
            "int" => self.encode_int(&instr.operands),
            "jmp" => self.encode_jmp(&instr.operands),
            "je" | "jz" => self.encode_jcc(0x74, &instr.operands),
            "jne" | "jnz" => self.encode_jcc(0x75, &instr.operands),
            "jl" => self.encode_jcc(0x7C, &instr.operands),
            "jg" => self.encode_jcc(0x7F, &instr.operands),
            "lea" => self.encode_lea(&instr.operands),
            _ => Vec::new(),
        };

        if let Some(section) = self.sections.get_mut(&self.current_section) {
            section.data.extend(bytes);
        }
    }

    fn encode_mov(&self, ops: &[Operand]) -> Vec<u8> {
        if ops.len() != 2 {
            return Vec::new();
        }

        match (&ops[0], &ops[1]) {
            (Operand::Register(dst), Operand::Register(src)) => {
                vec![0x89, 0xC0 | (src << 3) | dst]
            }
            (Operand::Register(dst), Operand::Immediate(imm)) => {
                let mut bytes = vec![0xB8 + dst];
                bytes.extend_from_slice(&(*imm as u32).to_le_bytes());
                bytes
            }
            (Operand::Memory(_), Operand::Immediate(imm)) => {
                let mut bytes = vec![0xC7];
                bytes.push(0x05);
                bytes.extend_from_slice(&0u32.to_le_bytes());
                bytes.extend_from_slice(&(*imm as u32).to_le_bytes());
                bytes
            }
            _ => Vec::new(),
        }
    }

    fn encode_alu(&self, opcode: u8, ops: &[Operand]) -> Vec<u8> {
        if ops.len() != 2 {
            return Vec::new();
        }

        match (&ops[0], &ops[1]) {
            (Operand::Register(dst), Operand::Register(src)) => {
                vec![0x01 + (opcode << 3), 0xC0 | (src << 3) | dst]
            }
            (Operand::Register(dst), Operand::Immediate(imm)) => {
                vec![0x81, 0xC0 + opcode + dst, *imm as u8, (*imm >> 8) as u8,
                     (*imm >> 16) as u8, (*imm >> 24) as u8]
            }
            _ => Vec::new(),
        }
    }

    fn encode_push(&self, ops: &[Operand]) -> Vec<u8> {
        if ops.is_empty() {
            return Vec::new();
        }

        match &ops[0] {
            Operand::Register(reg) => vec![0x50 + reg],
            Operand::Immediate(imm) => {
                let mut bytes = vec![0x68];
                bytes.extend_from_slice(&(*imm as u32).to_le_bytes());
                bytes
            }
            _ => Vec::new(),
        }
    }

    fn encode_pop(&self, ops: &[Operand]) -> Vec<u8> {
        if ops.is_empty() {
            return Vec::new();
        }

        match &ops[0] {
            Operand::Register(reg) => vec![0x58 + reg],
            _ => Vec::new(),
        }
    }

    fn encode_call(&self, _ops: &[Operand]) -> Vec<u8> {
        vec![0xE8, 0x00, 0x00, 0x00, 0x00]
    }

    fn encode_int(&self, ops: &[Operand]) -> Vec<u8> {
        if ops.is_empty() {
            return Vec::new();
        }

        match &ops[0] {
            Operand::Immediate(imm) => vec![0xCD, *imm as u8],
            _ => Vec::new(),
        }
    }

    fn encode_jmp(&self, ops: &[Operand]) -> Vec<u8> {
        vec![0xE9, 0x00, 0x00, 0x00, 0x00]
    }

    fn encode_jcc(&self, opcode: u8, ops: &[Operand]) -> Vec<u8> {
        vec![opcode, 0x00]
    }

    fn encode_lea(&self, ops: &[Operand]) -> Vec<u8> {
        if ops.len() != 2 {
            return Vec::new();
        }

        vec![0x8D, 0x05, 0x00, 0x00, 0x00, 0x00]
    }

    fn write_elf(&self, filename: &str) -> std::io::Result<()> {
        let mut file = File::create(filename)?;

        // ELF header
        let mut elf_header = ElfHeader {
            e_ident: [0x7F, b'E', b'L', b'F', 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            e_type: ET_REL,
            e_machine: EM_386,
            e_version: 1,
            e_entry: 0,
            e_phoff: 0,
            e_shoff: 0,
            e_flags: 0,
            e_ehsize: 52,
            e_phentsize: 0,
            e_phnum: 0,
            e_shentsize: 40,
            e_shnum: 0,
            e_shstrndx: 0,
        };

        // Write header placeholder
        file.write_all(unsafe {
            std::slice::from_raw_parts(
                &elf_header as *const _ as *const u8,
                std::mem::size_of::<ElfHeader>(),
            )
        })?;

        // Write sections
        let mut section_offset = 52;
        for section in self.sections.values() {
            file.write_all(&section.data)?;
            section_offset += section.data.len();
        }

        Ok(())
    }
}

#[no_mangle]
pub extern "C" fn assemble_to_object(
    _input: *const c_char,
    data: *const u8,
    data_len: c_int,
    output: *const c_char,
) -> c_int {
    let output_path = unsafe {
        CStr::from_ptr(output).to_string_lossy().to_string()
    };

    let mut asm = Assembler::new();
    
    // Deserialize and process statements
    // (Simplified - would parse the serialized data from Haskell)
    
    match asm.write_elf(&output_path) {
        Ok(_) => 0,
        Err(_) => 1,
    }
}
