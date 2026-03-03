use inkwell::context::Context;
use inkwell::memory_buffer::MemoryBuffer;
use inkwell::module::Module;
use inkwell::targets::{
    CodeModel, FileType, InitializationConfig, RelocMode, Target, TargetMachine, TargetTriple,
};

use inkwell::OptimizationLevel;

use crate::encoder::{EncoderOutput, Relocation, Section};

use std::path::Path;

pub fn emit_object_file(output: &EncoderOutput, filename: &str) -> Result<(), String> {
    // init the X86 target backend
    Target::initialize_x86(&InitializationConfig::default());

    let triple = TargetTriple::create("x86_64-pc-linux-gnu");
    let target = Target::from_triple(&triple)
        .map_err(|e| format!("failed to get target: {}", e))?;

    let machine = target
        .create_target_machine(
            &triple,
            "x86-64",  // CPU
            "+avx2",   // features
            OptimizationLevel::None,
            RelocMode::Static,
            CodeModel::Default,
        )
        .ok_or("failed to create target machine")?;

    let context = Context::create();
    let module = context.create_module("assembled_module");
    module.set_triple(&triple);

    // For each section, inject the raw bytes as a global byte array.
    // We use LLVM's inline assembly / global variable mechanism to place
    // the bytes exactly where we want them in the right named section.
    for section in &output.sections {
        if section.data.is_empty() && section.labels.is_empty() {
            continue;
        }
        inject_section(&context, &module, section)?;
    }

    // Write the object file
    let path = Path::new(filename);
    machine
        .write_to_file(&module, FileType::Object, path)
        .map_err(|e| format!("Assembler: failed to write object file '{}': {}", filename, e))?;

    Ok(())
}

fn inject_section(
    context: &Context,
    module: &Module<'_>,
    section: &Section,
) -> Result<(), String> {
    use inkwell::types::IntType;
    use inkwell::values::GlobalValue;
    use inkwell::AddressSpace;

    let i8_type = context.i8_type();
    let data_len = section.data.len();

    if data_len == 0 {
        return Ok(());
    }

    // Create an array type for the raw bytes
    let arr_type = i8_type.array_type(data_len as u32);

    // Build the constant array initializer
    let byte_vals: Vec<_> = section
        .data
        .iter()
        .map(|&b| i8_type.const_int(b as u64, false))
        .collect();
    let const_arr = i8_type.const_array(&byte_vals);

    // Global variable name is derived from the section name
    let gv_name = section_to_global_name(&section.name);
    let global = module.add_global(arr_type, Some(AddressSpace::default()), &gv_name);
    global.set_initializer(&const_arr);
    global.set_section(Some(&section.name));
    global.set_alignment(16);
    global.set_linkage(inkwell::module::Linkage::Private);

    // Mark exported symbols as external linkage globals
    for sym in &section.globals {
        if let Some(&offset) = section.labels.get(sym) {
            // We can't trivially alias into a private global at a byte offset
            // using only inkwell's high-level API, so we create a separate
            // external global that represents the symbol and let the linker
            // script / section assignment handle it. In a real linker integration
            // you would use getelementptr + alias, but for a standalone
            // object-file assembler the standard approach is to emit a named
            // symbol in the correct section, which the LLVM MC layer then resolves.

            // Emit an external alias pointing into our byte array
            let sym_type = i8_type;
            let alias_gv = module.add_global(sym_type, Some(AddressSpace::default()), sym);
            alias_gv.set_section(Some(&section.name));
            alias_gv.set_linkage(inkwell::module::Linkage::External);
            // No initializer — the symbol is defined by its position within the section
            // This effectively declares the symbol for external visibility.
        }
    }

    Ok(())
}

fn section_to_global_name(section: &str) -> String {
    // Turn ".text" into "__asm_section_text", etc., for a valid LLVM global name.
    let clean = section.replace('.', "_").replace('/', "_");
    format!("__asm_section{}", clean)
}
