The `default_assembler.bin` file holds a single bit (`1`/`0`), that is used to represent the default assembler on the users system. 

- `0` represents the C++ assembler that produces a direct ELF binary.
- `1` represents the Rust assemlber that produces a LLVM object file\


*if `1` is selected then the compiler might use a custom linker if not it will just produce a object file*