# Compilers and flags
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -g -std=c11
CXXFLAGS = -Wall -Wextra -g -std=c++17
TARGET = craw

# rs assembler library 
RUST_SRC_DIR   = src/assembler/rust_src
RUST_TARGET    = $(RUST_SRC_DIR)/target/release
RUST_LIB       = $(RUST_TARGET)/libassembler.a

# Detect LLVM version installed (prefer llvm-config on PATH)
LLVM_CONFIG    ?= llvm-config
LLVM_LDFLAGS   := $(shell $(LLVM_CONFIG) --ldflags 2>/dev/null)
LLVM_LIBS      := $(shell $(LLVM_CONFIG) --libs all 2>/dev/null)
LLVM_SYSLIBS   := $(shell $(LLVM_CONFIG) --system-libs 2>/dev/null)

# C/C++ src
C_SOURCES = src/main.c \
            src/throwErr.c \
            src/lexer/token.c \
            src/lexer/vector.c \
            src/lexer/f32.c \
            src/lexer/f64.c \
            src/lexer/lexer.c \
            src/tag/tag.c \
            src/preprocess/preprocessor.c\
            src/parser/AST_vector.c \
            src/parser/AST.c \
            src/parser/hashmap.c \
            src/parser/parser.c

CXX_SOURCES = src/assembler/assembler.cpp \
              src/assembler/emit.cpp \
              src/assembler/encode.cpp \
              src/assembler/mainAssembler.cpp \
              src/assembler/parse.cpp

# Object files
C_OBJECTS   = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
OBJECTS     = $(C_OBJECTS) $(CXX_OBJECTS)

# default target
.PHONY: all clean rust-lib rust-clean

all: rust-lib $(TARGET)

# build the rs assembler static library
rust-lib: $(RUST_LIB)

$(RUST_LIB): $(shell find $(RUST_SRC_DIR)/src -name '*.rs') $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] building Rust assembler library..."
	cargo build --release --manifest-path $(RUST_SRC_DIR)/Cargo.toml
	@echo "[cargo] done → $(RUST_LIB)"

# link
# The Rust staticlib + LLVM libs must come after the C/C++ objects so the
# linker can resolve all symbols.
$(TARGET): $(OBJECTS) $(RUST_LIB)
	$(CXX) $(CXXFLAGS) $(OBJECTS) \
	    -L$(RUST_TARGET) -lassembler \
	    $(LLVM_LDFLAGS) $(LLVM_LIBS) $(LLVM_SYSLIBS) \
	    -lpthread -ldl -lm \
	    -o $(TARGET)

# compile C src
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# compile C++ src
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean
clean:
	rm -f $(TARGET) $(OBJECTS)

rust-clean:
	cargo clean --manifest-path $(RUST_SRC_DIR)/Cargo.toml
