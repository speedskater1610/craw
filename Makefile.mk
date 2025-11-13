CC = gcc
CXX = g++
CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -std=c++17

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

CXX_SOURCES = src/assembler/assembler.cpp \
              src/assembler/emit.cpp \
              src/assembler/encode.cpp \
              src/assembler/mainAssembler.cpp \
              src/assembler/parse.cpp


C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
