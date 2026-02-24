use std::collections::HashMap;

/// A parsed operand.
#[derive(Debug, Clone, PartialEq)]
pub enum Operand {
    /// Register (e.g. rax, eax, al)
    Register(String),
    /// Immediate value (e.g. $42, 0xFF)
    Immediate(i64),
    /// Memory reference: displacement(base, index, scale)
    Memory {
        displacement: i64,
        base: Option<String>,
        index: Option<String>,
        scale: u8,
    },
    /// Label reference
    Label(String),
}

/// A single assembled line.
#[derive(Debug, Clone)]
pub struct Instruction {
    /// Optional label on this line
    pub label: Option<String>,
    /// Mnemonic (lowercase), e.g. "mov", "add", "ret"
    pub mnemonic: Option<String>,
    /// Operands (left-to-right, destination first for Intel-style)
    pub operands: Vec<Operand>,
}

/// Directive kinds we care about.
#[derive(Debug, Clone)]
pub enum Directive {
    Section(String),
    Global(String),
    Byte(Vec<i64>),
    Word(Vec<i64>),
    Long(Vec<i64>),
    Quad(Vec<i64>),
    Ascii(String),
    Asciz(String),
    Zero(usize),
}

/// A top-level parsed item.
#[derive(Debug, Clone)]
pub enum Item {
    Instruction(Instruction),
    Directive(Directive),
}

pub fn parse(source: &str) -> Result<Vec<Item>, String> {
    let mut items = Vec::new();

    for (line_no, raw_line) in source.lines().enumerate() {
        // Strip comments (both ; and # styles)
        let line = strip_comment(raw_line).trim().to_string();
        if line.is_empty() {
            continue;
        }

        // Check for directive
        if let Some(stripped) = line.strip_prefix('.') {
            match parse_directive(stripped) {
                Ok(d) => items.push(Item::Directive(d)),
                Err(e) => return Err(format!("line {}: {}", line_no + 1, e)),
            }
            continue;
        }

        // Check for label (ends with ':')
        let (label, rest) = split_label(&line);
        let rest = rest.trim();

        if rest.is_empty() {
            // Label-only line
            if let Some(lbl) = label {
                items.push(Item::Instruction(Instruction {
                    label: Some(lbl),
                    mnemonic: None,
                    operands: vec![],
                }));
            }
            continue;
        }

        // Parse mnemonic + operands
        let instr = parse_instruction(label, rest)
            .map_err(|e| format!("line {}: {}", line_no + 1, e))?;
        items.push(Item::Instruction(instr));
    }

    Ok(items)
}

fn strip_comment(line: &str) -> &str {
    // Handle both ';' and '#' comment characters
    for (i, c) in line.char_indices() {
        if c == ';' || c == '#' {
            return &line[..i];
        }
        // Also handle // style
        if c == '/' {
            let remaining = &line[i..];
            if remaining.starts_with("//") {
                return &line[..i];
            }
        }
    }
    line
}

fn split_label(line: &str) -> (Option<String>, &str) {
    // A label is an identifier followed by ':'
    if let Some(colon_pos) = line.find(':') {
        let potential_label = &line[..colon_pos];
        if is_valid_identifier(potential_label.trim()) {
            return (
                Some(potential_label.trim().to_string()),
                &line[colon_pos + 1..],
            );
        }
    }
    (None, line)
}

fn is_valid_identifier(s: &str) -> bool {
    if s.is_empty() {
        return false;
    }
    let mut chars = s.chars();
    let first = chars.next().unwrap();
    if !first.is_ascii_alphabetic() && first != '_' && first != '.' {
        return false;
    }
    chars.all(|c| c.is_ascii_alphanumeric() || c == '_' || c == '.')
}

fn parse_instruction(label: Option<String>, text: &str) -> Result<Instruction, String> {
    let mut parts = text.splitn(2, |c: char| c.is_ascii_whitespace());
    let mnemonic = parts
        .next()
        .map(|s| s.trim().to_lowercase())
        .filter(|s| !s.is_empty());

    let operands_str = parts.next().unwrap_or("").trim();
    let operands = if operands_str.is_empty() {
        vec![]
    } else {
        parse_operands(operands_str)?
    };

    Ok(Instruction {
        label,
        mnemonic,
        operands,
    })
}

fn parse_operands(s: &str) -> Result<Vec<Operand>, String> {
    // Split on ',' but not inside parentheses
    let mut operands = Vec::new();
    let mut depth = 0usize;
    let mut current = String::new();

    for c in s.chars() {
        match c {
            '(' => {
                depth += 1;
                current.push(c);
            }
            ')' => {
                if depth == 0 {
                    return Err("RASSEMBLER error: unmatched ')'".to_string());
                }
                depth -= 1;
                current.push(c);
            }
            ',' if depth == 0 => {
                operands.push(parse_operand(current.trim())?);
                current = String::new();
            }
            _ => current.push(c),
        }
    }
    if !current.trim().is_empty() {
        operands.push(parse_operand(current.trim())?);
    }
    Ok(operands)
}

fn parse_operand(s: &str) -> Result<Operand, String> {
    let s = s.trim();

    // Immediate: starts with '$'
    if let Some(imm_str) = s.strip_prefix('$') {
        let val = parse_integer(imm_str)?;
        return Ok(Operand::Immediate(val));
    }

    // Register: starts with '%'
    if let Some(reg_str) = s.strip_prefix('%') {
        return Ok(Operand::Register(reg_str.to_lowercase()));
    }

    // Memory: contains '(' — e.g. 8(%rsp) or (%rbx,%rax,4)
    if s.contains('(') {
        return parse_memory_operand(s);
    }

    // Plain integer (no '%' or '$'): treat as immediate
    if s.starts_with(|c: char| c.is_ascii_digit() || c == '-') {
        let val = parse_integer(s)?;
        return Ok(Operand::Immediate(val));
    }

    // Intel-style register name (no prefix)?
    if is_register_name(s) {
        return Ok(Operand::Register(s.to_lowercase()));
    }

    // Otherwise treat as label reference
    Ok(Operand::Label(s.to_string()))
}

fn parse_memory_operand(s: &str) -> Result<Operand, String> {
    // Format: disp(base, index, scale)  or  disp(base)
    let paren_open = s.find('(').ok_or("expected '('")?;
    let paren_close = s.rfind(')').ok_or("expected ')'")?;

    let disp_str = s[..paren_open].trim();
    let displacement: i64 = if disp_str.is_empty() {
        0
    } else {
        parse_integer(disp_str)?
    };

    let inner = &s[paren_open + 1..paren_close];
    let parts: Vec<&str> = inner.split(',').collect();

    let base = parts
        .get(0)
        .map(|r| r.trim().trim_start_matches('%').to_lowercase())
        .filter(|r| !r.is_empty());

    let index = parts
        .get(1)
        .map(|r| r.trim().trim_start_matches('%').to_lowercase())
        .filter(|r| !r.is_empty());

    let scale: u8 = if let Some(sc_str) = parts.get(2) {
        sc_str.trim().parse::<u8>().map_err(|_| "invalid scale")?
    } else {
        1
    };

    Ok(Operand::Memory {
        displacement,
        base,
        index,
        scale,
    })
}

fn parse_integer(s: &str) -> Result<i64, String> {
    let s = s.trim();
    if let Some(hex) = s.strip_prefix("0x").or_else(|| s.strip_prefix("0X")) {
        i64::from_str_radix(hex, 16).map_err(|_| format!("invalid hex integer: {}", s))
    } else if let Some(hex) = s.strip_prefix("-0x").or_else(|| s.strip_prefix("-0X")) {
        i64::from_str_radix(hex, 16)
            .map(|v| -v)
            .map_err(|_| format!("invalid hex integer: {}", s))
    } else {
        s.parse::<i64>()
            .map_err(|_| format!("invalid integer: {}", s))
    }
}

fn parse_directive(s: &str) -> Result<Directive, String> {
    let mut parts = s.splitn(2, |c: char| c.is_ascii_whitespace());
    let name = parts.next().unwrap_or("").trim().to_lowercase();
    let args = parts.next().unwrap_or("").trim();

    match name.as_str() {
        "section" | "text" | "data" | "bss" => {
            let sec_name = if name == "section" {
                args.trim_matches('"').to_string()
            } else {
                format!(".{}", name)
            };
            Ok(Directive::Section(sec_name))
        }
        "globl" | "global" => Ok(Directive::Global(args.to_string())),
        "byte" => Ok(Directive::Byte(parse_int_list(args)?)),
        "word" | "short" => Ok(Directive::Word(parse_int_list(args)?)),
        "long" | "int" => Ok(Directive::Long(parse_int_list(args)?)),
        "quad" => Ok(Directive::Quad(parse_int_list(args)?)),
        "ascii" => Ok(Directive::Ascii(parse_string_literal(args)?)),
        "asciz" | "string" => Ok(Directive::Asciz(parse_string_literal(args)?)),
        "zero" | "space" => {
            let n = args.trim().parse::<usize>().map_err(|_| "invalid .zero count")?;
            Ok(Directive::Zero(n))
        }
        _ => Err(format!("unknown directive: .{}", name)),
    }
}

fn parse_int_list(s: &str) -> Result<Vec<i64>, String> {
    s.split(',')
        .map(|v| parse_integer(v.trim()))
        .collect()
}

fn parse_string_literal(s: &str) -> Result<String, String> {
    let s = s.trim();
    if s.starts_with('"') && s.ends_with('"') {
        let inner = &s[1..s.len() - 1];
        // Handle basic escape sequences
        let mut result = String::new();
        let mut chars = inner.chars().peekable();
        while let Some(c) = chars.next() {
            if c == '\\' {
                match chars.next() {
                    Some('n') => result.push('\n'),
                    Some('t') => result.push('\t'),
                    Some('r') => result.push('\r'),
                    Some('0') => result.push('\0'),
                    Some('\\') => result.push('\\'),
                    Some('"') => result.push('"'),
                    Some(other) => {
                        result.push('\\');
                        result.push(other);
                    }
                    None => return Err("unterminated escape sequence".to_string()),
                }
            } else {
                result.push(c);
            }
        }
        Ok(result)
    } else {
        Err(format!("expected quoted string, got: {}", s))
    }
}

fn is_register_name(s: &str) -> bool {
    const REGS: &[&str] = &[
        // 64-bit
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp",
        "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
        "rip",
        // 32-bit
        "eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp",
        "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
        // 16-bit
        "ax", "bx", "cx", "dx", "si", "di", "sp", "bp",
        // 8-bit
        "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh",
        "sil", "dil", "spl", "bpl",
        // Segment
        "cs", "ds", "es", "fs", "gs", "ss",
        // XMM/YMM
        "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7",
        "xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15",
        "ymm0","ymm1","ymm2","ymm3","ymm4","ymm5","ymm6","ymm7",
    ];
    REGS.contains(&s.to_lowercase().as_str())
}
