# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# scr and object files
SRC = main.c lexer.c token.c vector.c
OBJ = $(SRC:.c=.o)
TARGET = craw

# Default target
all: $(TARGET)

# Link object files into the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile each .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)
