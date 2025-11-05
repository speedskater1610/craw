CC = gcc
CFLAGS = -Wall -g
TARGET = craw
SOURCES = src/main.c src/throwErr.c src/lexer/token.c src/lexer/vector.c src/lexer/f32.c src/lexer/f64.c src/lexer/lexer.c
OBJECTS = $(SOURCES:.c=.o) # Automatically creates object files

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
