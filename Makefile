# Output binary
TARGET = craw.exe

# Compiler and flags
CXX = g++
CXXFLAGS = -O2

# Source files
C_SOURCES = main.c read/read.c write/write.c builder/build.c
CPP_SOURCES = std_compile/std.cpp

# Object files
OBJECTS = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o)

# Default rule
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# Compile C source files
%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
