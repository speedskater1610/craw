# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Executable name
TARGET := crasm

# Source files
SRCS := main.cpp \
		parse.cpp \
		assembler.cpp \
		emit.cpp \
		encode.cpp

# Object files (replace .cpp with .o)
OBJS := $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp into .o
%.o: %.cpp Assembler.hpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning up..."
	rm -f $(OBJS) $(TARGET)

# Run the program (optional)
run: $(TARGET)
	./$(TARGET)

# Rebuild everything
rebuild: clean all

.PHONY: all clean run rebuild
