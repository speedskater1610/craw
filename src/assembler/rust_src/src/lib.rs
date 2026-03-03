mod parser;
mod encoder;
mod codegen;

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::path::Path;

/// Assemble x86 assembly source into an LLVM object file.
///
/// # Safety
/// `source` and `filename` must be valid, non-null, null-terminated C strings.
/// The returned pointer is heap-allocated and must be freed by calling `assemble_free_error`.
/// Returns a null pointer on success, or a pointer to an error string on failure.
#[no_mangle]
pub unsafe extern "C" fn assemble(
    source: *const c_char,
    filename: *const c_char,
) -> *mut c_char {
    // Convert C strings to Rust strings
    let source_str = match unsafe { CStr::from_ptr(source) }.to_str() {
        Ok(s) => s,
        Err(_) => return error_cstring("invalid UTF-8 in source string"),
    };
    let filename_str = match unsafe { CStr::from_ptr(filename) }.to_str() {
        Ok(s) => s,
        Err(_) => return error_cstring("invalid UTF-8 in filename"),
    };

    match assemble_impl(source_str, filename_str) {
        Ok(()) => std::ptr::null_mut(),
        Err(e) => error_cstring(&e),
    }
}

/// Free an error string returned by `assemble`.
///
/// # Safety
/// `ptr` must be a pointer previously returned by `assemble` (non-null).
#[no_mangle]
pub unsafe extern "C" fn assemble_free_error(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe { drop(CString::from_raw(ptr)) };
    }
}

fn error_cstring(msg: &str) -> *mut c_char {
    // Truncate any embedded nuls for safety
    let sanitized: String = msg.chars().filter(|&c| c != '\0').collect();
    CString::new(sanitized)
        .unwrap_or_else(|_| CString::new("unknown error").unwrap())
        .into_raw()
}

fn assemble_impl(source: &str, filename: &str) -> Result<(), String> {
    // 1. Parse the assembly source into an instruction list
    let instructions = parser::parse(source)?;

    // 2. Encode instructions into machine code bytes
    let sections = encoder::encode(&instructions)?;

    // 3. Use LLVM (via inkwell) to produce an object file
    codegen::emit_object_file(&sections, filename)?;

    Ok(())
}
