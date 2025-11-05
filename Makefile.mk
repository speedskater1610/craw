CC = gcc
CFLAGS = -Wall -g
TARGET = craw
SOURCES = src/main.c src/f32.c src/f64.c src/lexer.c src/throwErr.c src/token.c src/vector.c
OBJECTS = $(SOURCES:.c=.o) # Automatically creates object files

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
