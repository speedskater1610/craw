// src/assembler/rust_src/src/codegen.rs
// Uses the `inkwell` LLVM binding to produce a native object file
// from raw section data produced by the encoder.

use inkwell::context::Context;
use inkwell::module::Module;
use inkwell::targets::{
    CodeModel, FileType, InitializationConfig, RelocMode, Target, TargetTriple,
};
use inkwell::OptimizationLevel;

use crate::encoder::{EncoderOutput, Section};

use std::path::Path;

pub fn emit_object_file(output: &EncoderOutput, filename: &str) -> Result<(), String> {
    Target::initialize_x86(&InitializationConfig::default());

    let triple = TargetTriple::create("x86_64-pc-linux-gnu");
    let target = Target::from_triple(&triple)
        .map_err(|e| format!("failed to get target: {}", e))?;

    let machine = target
        .create_target_machine(
            &triple,
            "x86-64",
            "+avx2",
            OptimizationLevel::None,
            RelocMode::Static,
            CodeModel::Default,
        )
        .ok_or("failed to create target machine")?;

    let context = Context::create();
    let module = context.create_module("assembled_module");
    module.set_triple(&triple);

    for section in &output.sections {
        if section.data.is_empty() && section.labels.is_empty() {
            continue;
        }
        inject_section(&context, &module, section)?;
    }

    let path = Path::new(filename);
    machine
        .write_to_file(&module, FileType::Object, path)
        .map_err(|e| format!("failed to write object file '{}': {}", filename, e))?;

    Ok(())
}

// Named lifetime 'ctx ties the Context borrow to the Module's lifetime parameter,
// satisfying inkwell's invariance requirement on Module<'ctx>.
fn inject_section<'ctx>(
    context: &'ctx Context,
    module: &Module<'ctx>,
    section: &Section,
) -> Result<(), String> {
    use inkwell::AddressSpace;

    let i8_type = context.i8_type();
    let data_len = section.data.len();

    if data_len == 0 {
        return Ok(());
    }

    let arr_type = i8_type.array_type(data_len as u32);

    let byte_vals: Vec<_> = section
        .data
        .iter()
        .map(|&b| i8_type.const_int(b as u64, false))
        .collect();
    let const_arr = i8_type.const_array(&byte_vals);

    let gv_name = section_to_global_name(&section.name);
    let global = module.add_global(arr_type, Some(AddressSpace::default()), &gv_name);
    global.set_initializer(&const_arr);
    global.set_section(Some(&section.name));
    global.set_alignment(16);
    global.set_linkage(inkwell::module::Linkage::Private);

    // Expose .globl symbols as external linkage globals in the same section.
    for sym in &section.globals {
        if section.labels.contains_key(sym) {
            let alias_gv = module.add_global(i8_type, Some(AddressSpace::default()), sym);
            alias_gv.set_section(Some(&section.name));
            alias_gv.set_linkage(inkwell::module::Linkage::External);
        }
    }

    Ok(())
}

fn section_to_global_name(section: &str) -> String {
    let clean = section.replace('.', "_").replace('/', "_");
    format!("__asm_section{}", clean)
}
