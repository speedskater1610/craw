# Compilers and flags
CC       = gcc
CXX      = g++
CFLAGS   = -Wall -Wextra -g -std=c11
CXXFLAGS = -Wall -Wextra -g -std=c++17
TARGET   = crawc
PANTRY   := $(shell command -v pantry 2> /dev/null)

# -----------------------------------------------------------------------
# Rust / LLVM assembler (optional; only needed if you want the Rust
# backend selected via ~/.config/craw/which_assembler.bin = 1).
# Run `make all` for the full build (requires cargo + llvm-config).
# Run `make quick` to build without Rust/LLVM using the C++ backend only.
# -----------------------------------------------------------------------
RUST_SRC_DIR   = src/assembler/rust_src
RUST_TARGET    = $(RUST_SRC_DIR)/target/release
RUST_LIB       = $(RUST_TARGET)/libassembler.a

LLVM_CONFIG    ?= llvm-config
LLVM_LDFLAGS   := $(shell $(LLVM_CONFIG) --ldflags 2>/dev/null)
LLVM_LIBS      := $(shell $(LLVM_CONFIG) --libs all 2>/dev/null)
LLVM_SYSLIBS   := $(shell $(LLVM_CONFIG) --system-libs 2>/dev/null)

# -----------------------------------------------------------------------
# Zig CLI
# Build separately as its own executable
# removed if built with `clean`
# -----------------------------------------------------------------------
ZIG_SRC_DIR = CLI
ZIGC = zig

# -----------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------
C_SOURCES = \
    src/main.c \
    src/setup.c \
    src/read.c \
    src/write.c \
    src/throwErr.c \
    src/assemble.c \
    src/lexer/token.c \
    src/lexer/vector.c \
    src/lexer/f32.c \
    src/lexer/f64.c \
    src/lexer/lexer.c \
    src/tag/tag.c \
    src/preprocess/preprocessor.c \
    src/preprocess/func_exists.c \
    src/parser/AST.c \
    src/parser/hashmap.c \
    src/parser/parser.c \
    src/codegen/elf32/elf32_codegen.c \
    src/codegen/x86-64/x86-64_codegen.c

CXX_SOURCES = \
    src/assembler/assembler.cpp \
    src/assembler/emit.cpp \
    src/assembler/encode.cpp \
    src/assembler/mainAssembler.cpp \
    src/assembler/parse.cpp

# Stub replaces Rust FFI symbols when building without the Rust lib
STUB_SOURCES = src/assembler/rassembler_stub.c

C_OBJECTS    = $(C_SOURCES:.c=.o)
CXX_OBJECTS  = $(CXX_SOURCES:.cpp=.o)
STUB_OBJECTS = $(STUB_SOURCES:.c=.o)
ALL_OBJECTS  = $(C_OBJECTS) $(CXX_OBJECTS)

# -----------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------
.PHONY: all quick clean rust-lib rust-clean cli test

# Full build: requires cargo + LLVM
all: rust-lib $(TARGET)

# Quick build: C++ assembler backend only, no Rust/LLVM needed
quick: $(C_OBJECTS) $(CXX_OBJECTS) $(STUB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(C_OBJECTS) $(CXX_OBJECTS) $(STUB_OBJECTS) \
	    -lpthread -ldl -lm \
	    -o $(TARGET)
	@echo "Built $(TARGET) (quick mode (C++ assembler backend))"

# Build Rust static library
rust-lib: $(RUST_LIB)

$(RUST_LIB): $(shell find $(RUST_SRC_DIR)/src -name '*.rs') $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] building Rust assembler library..."
	cargo build --release --manifest-path $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] done -> $(RUST_LIB)"

# Full link (with Rust lib)
$(TARGET): $(ALL_OBJECTS) $(RUST_LIB)
	$(CXX) $(CXXFLAGS) $(ALL_OBJECTS) \
	    -L$(RUST_TARGET) -lassembler \
	    $(LLVM_LDFLAGS) $(LLVM_LIBS) $(LLVM_SYSLIBS) \
	    -lpthread -ldl -lm \
	    -o $(TARGET)

# Compile rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the CLI
cli:
	@if [ -z "$(PANTRY)" ]; then \
		echo "Pantry is NOT installed. Please install it first."; \
		exit 1; \
	fi
	cd $(ZIG_SRC_DIR) && pantry install && $(ZIGC) build

# Run test suite
TEST_FILES = $(wildcard tests/*.craw)
test: quick
	@pass=0; fail=0; \
	for f in $(TEST_FILES); do \
	    out=$$(./$(TARGET) -o /dev/null "$$f" 2>&1); \
	    if echo "$$out" | grep -q "Binary written"; then \
	        echo "  PASS: $$f"; pass=$$((pass+1)); \
	    else \
	        echo "  FAIL: $$f"; echo "$$out" | grep -E "Error|error|failed" | head -2 | sed 's/^/    /'; fail=$$((fail+1)); \
	    fi; \
	done; \
	echo ""; echo "Results: $$pass passed, $$fail failed"; \
	[ $$fail -eq 0 ]

# Clean
clean:
	@echo "Removing C & C++ build"; \
	rm -f $(TARGET) $(ALL_OBJECTS) $(STUB_OBJECTS); \
	echo "Removing Rust assembler"; \
	if [ -f src/assembler/rust_src/Cargo.toml ]; then \
		cargo clean --manifest-path src/assembler/rust_src/Cargo.toml; \
	else \
		echo "Assembler: Cargo.toml not found, skipping cargo clean."; \
	fi; \
	echo "Removing Zig CLI"; \
	if [ -d "$(ZIG_SRC_DIR)/zig-out" ] || [ -d "$(ZIG_SRC_DIR)/.zig-cache" ]; then \
		echo "Removing Zig CLI build"; \
		rm -rf $(ZIG_SRC_DIR)/zig-out $(ZIG_SRC_DIR)/.zig-cache; \
	else \
		echo "CLI build directory does not exist."; \
	fi
