# Craw Compiler Style & Syntax Guidelines

This document defines coding conventions for the `craw` compiler project. Following these ensures consistent, readable, and maintainable code across contributors.

---

## 1. General Rules

- Programming language: **C**
- Indentation: **4 spaces per tab** (no hard tabs)
- Maximum line length: **120 characters**
- Brace style: **K&R style**

Example:
```c
    if (condition) {
        doSomething();
    } else {
        doSomethingElse();
    }
```
- Keep functions reasonably short (~50–80 lines if possible)
- Avoid global variables unless strictly necessary

---

## 2. Naming Conventions

| Item                  | Convention  | Example                    |
|-----------------------|-------------|-----------------------------|
| Filenames             | camelCase   | lexer.c, tokenizer.c        |
| Structs / Types       | PascalCase  | Lexer, ResultSigTok         |
| Functions / Variables | snake_case  | next_token(), current_char  |
| Constants / Enums     | ALL_CAPS    | Eof, LeftParen              |

---

## 3. Code Organization

- Each module has its `.c` and `.h` files (e.g., lexer.c / lexer.h)
- Functions grouped by functionality
- Forward declarations go in headers; implementation in `.c`
- Keep helper/private functions `static` inside `.c` files

---

## 4. Memory & Error Handling

- Use `malloc` / `realloc` carefully; always check for `NULL`
- Free dynamically allocated memory when no longer needed
- For errors:
  - Use `Err()` function to print lexer/compiler errors
  - Or return a `ResultSigTok` with `Error = true`

---

## 5. Functions

- Constructors: `Type* Type_new(...)`
- Token processing: `ResultSigTok next_token(Lexer* self)`
- Modifiers: functions that modify structs or buffers take pointers
- Functions should avoid side effects outside their intended scope

---

## 6. Comments & Documentation

- Document all public functions briefly
- Single-line comments (`//`) for inline notes
- Block comments (`/* ... */`) for longer explanations
- Explain *why* in comments, not just *what*

---

## 7. Formatting & Style

- Use consistent spacing:
```c
    a = b + c; // correct
    a=b+c;     // incorrect
```
- Strings: `"like this"`
- Characters: `'c'`
- Avoid single-letter variable names except in small loops
- Keep consistent spacing around operators and after commas

---

## 8. Examples

### Constructor
```c
    Lexer* Lexer_new(char* input) {
        Lexer* s = (Lexer*)malloc(sizeof(Lexer));
        if (s != NULL) {
            s->input = input;
            s->position = 0;
            s->line = 1;
            s->column = 1;
        }
        return s;
    }
```
### Tokenizer
```c
    Vector tokenize(Lexer* self) {
        Vector tokens = {0};

        while (!is_at_end(self)) {
            skip_whitespace(self);
            if (is_at_end(self)) break;

            ResultSigTok result = next_token(self);
            push(&tokens, result.token);
        }

        Token* eof_token = Token_new(Eof, "", self->line, self->column);
        push(&tokens, *eof_token);

        return tokens;
    }
```
### Conditional & Loop Example
```c
    char current_char(Lexer* self) {
        if (is_at_end(self)) {
            return '\0';
        } else {
            return self->input[self->position];
        }
    }

    void advance(Lexer* self) {
        if (!is_at_end(self)) {
            if (self->input[self->position] == '\n') {
                self->line += 1;
                self->column = 1;
            } else {
                self->column += 1;
            }
            self->position += 1;
        }
    }
```
---

## 9. Additional Notes

- Helper functions like `append_char()` should handle memory safely
- Always validate input when reading numbers, identifiers, strings, or chars
- Keep consistent naming conventions to maintain readability
- Use `ResultSigTok` consistently to return errors or tokens

---
