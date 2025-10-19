#include "lexer.h"

    
// constructor
Lexer* Lexer_new (char* input) {
    Lexer* s = (Lexer*)malloc(sizeof(Lexer));
    if (s != NULL) {
        s->input = input;
        s->position = 0;
        s->line = 1;
        s->column = 1;
    }
    return s;
}


Result tokenize(Lexer* self) {
    Vector* tokens = create_vector();
    
    while (is_at_end(self)) {}    // TODO finish thsi once implented all other functions
}

/*
fn read_string_literal(&mut self, start_line: usize, start_column: usize) -> Result<Token, String> {
        self.advance(); // consume opening "
        let mut string = String::new();

        while !self.is_at_end() && self.current_char() != '"' {
            if self.current_char() == '\\' {
                self.advance();
                if self.is_at_end() {
                    return Err(format!("Unterminated string literal at line {}, column {}", start_line, start_column));
                }

                let escaped = match self.current_char() {
                    'n' => '\n',
                    't' => '\t',
                    '0' => '\0',
                    '\\' => '\\',
                    '"' => '"',
                    c => c,
                };
                string.push(escaped);
            } else {
                string.push(self.current_char());
            }
            self.advance();
        }

        if self.is_at_end() {
            return Err(format!("Unterminated string literal at line {}, column {}", start_line, start_column));
        }

        self.advance(); // consume closing "

        Ok(Token::new(TokenType::StringLiteral(string.clone()), format!("\"{}\"", string), start_line, start_column))
    }
*/

Result read_string_literal(Lexer* self, unsigned int start_line, unsigned int start_column) {
    advance(self);
    
    char* string = "";
    
    while (is_at_end(self) && current_char(self) != '"') {
        if (current_char(self) == '\\') {
            advance(self);
            if is_at_end(self) {
                // return Err(format!("Unterminated string literal at line {}, column {}", start_line, start_column));
                Result Error = 
            }
        }
    }
}


void skip_whitespace (Lexer* self) {
    while (is_at_end(self)) {
        switch (current_char(self)) {
            case ' ':
                advance(self);
                break;
            case '\t':
                advance(self);
                break;
            case '\r':
                advance(self);
                break;
            case '\n':
                advance(self);
                break;
            default : break;
        }
    }
}

char current_char(Lexer* self) {
    if (is_at_end(self)) {
        return '\0';
    } else {
        return self->input[self->position];
    }
}

void advance (Lexer* self) {
    if (!is_at_end(self)) {
        if (self->input[self->position] == '\n') {
            self->line += 1;
            self->column = 1;
        } else {
            self->column += 1;
        }
        self-> position += 1;
    }
}

// bool
int is_at_end (Lexer* self) {
    return (self->position >= strlen(self->input)); 
}
