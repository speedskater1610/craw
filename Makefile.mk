# Compilers and flags
CC       = gcc
CXX      = g++
CFLAGS   = -Wall -Wextra -g -std=c11
CXXFLAGS = -Wall -Wextra -g -std=c++17
TARGET   = craw

# -----------------------------------------------------------------------
# Rust / LLVM assembler (optional — only needed if you want the Rust
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
    src/preprocess/funcExists.c \
    src/parser/AST_vector.c \
    src/parser/AST.c \
    src/parser/hashmap.c \
    src/parser/parser.c \
    src/codegen/codegen.c

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
.PHONY: all quick clean rust-lib rust-clean

# Full build: requires cargo + LLVM
all: rust-lib $(TARGET)

# Quick build: C++ assembler backend only, no Rust/LLVM needed
quick: $(C_OBJECTS) $(CXX_OBJECTS) $(STUB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(C_OBJECTS) $(CXX_OBJECTS) $(STUB_OBJECTS) \
	    -lpthread -ldl -lm \
	    -o $(TARGET)
	@echo "Built $(TARGET) (quick mode — C++ assembler backend)"

# Build Rust static library
rust-lib: $(RUST_LIB)

$(RUST_LIB): $(shell find $(RUST_SRC_DIR)/src -name '*.rs') $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] building Rust assembler library..."
	cargo build --release --manifest-path $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] done → $(RUST_LIB)"

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

# Run test suite
TEST_FILES = $(wildcard tests/*.craw)
.PHONY: test
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
	rm -f $(TARGET) $(ALL_OBJECTS) $(STUB_OBJECTS)

rust-clean: clean
	cargo clean --manifest-path $(RUST_SRC_DIR)/Cargo.toml