# CRASM Assembler Build System
# Haskell frontend + Rust backend

.PHONY: all clean frontend backend link

# Compiler flags
GHC = ghc
RUSTC = rustc
GHC_FLAGS = -O2 -outputdir build/haskell -i./src/haskell
RUST_FLAGS = --crate-type staticlib -C opt-level=2

# Directories
BUILD_DIR = build
SRC_HS = src/haskell
SRC_RS = src/rust
OBJ_DIR = $(BUILD_DIR)/obj

all: crasm

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/haskell $(BUILD_DIR)/rust $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build Haskell frontend
frontend: $(BUILD_DIR) $(OBJ_DIR)
	@echo "Haskell modules will be compiled during linking"

# Build Rust backend
backend: $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/rust
	$(RUSTC) $(RUST_FLAGS) $(SRC_RS)/backend.rs -o $(BUILD_DIR)/rust/libbackend.a

# Link everything together
crasm: frontend backend
	$(GHC) $(GHC_FLAGS) \
		$(SRC_HS)/Main.hs \
		$(BUILD_DIR)/rust/libbackend.a \
		-o crasm

clean:
	rm -rf $(BUILD_DIR) crasm
	rm -f *.hi *.o

install: crasm
	install -m 755 crasm /usr/local/bin/

test: crasm
	./crasm tests/test.asm test_output
	ld -m elf_i386 test_output.o -o test_output
	./test_output
