# compilers and flags
CC = gcc
CPP = g++
CFLAGS = -Wall -g
CPPFLAGS = -Wall -g

# Target and source files
TARGET = craw

C_SOURCES = src/main.c \
            src/throwErr.c \
            src/lexer/token.c \
            src/lexer/vector.c \
            src/lexer/f32.c \
            src/lexer/f64.c \
            src/lexer/lexer.c \
            src/tag/tag.c \
            src/preprocesser.c

CPP_SOURCES = src/assembler/assembler.cpp


# Automatically create object from file lists
C_OBJECTS = $(C_SOURCES:.c=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

# Combine all objects for linking
OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)

# Default target
all: $(TARGET)

# linking the final executable
$(TARGET): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) -o $(TARGET)

# compiling C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# compiling C++ files
%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

# Phony target for cleaning
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
