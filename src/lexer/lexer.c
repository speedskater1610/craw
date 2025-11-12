#include "lexer.h"

    
// constructor
Lexer* Lexer_new(char *input) {
    Lexer* s = (Lexer*)malloc(sizeof(Lexer));
    if (s != NULL) {
        s->input = input;
        s->position = 0;
        s->line = 1;
        s->column = 1;
    }
    return s;
}

Vector tokenize(Lexer *self) { 
    Vector tokens = {0};

    while (!is_at_end(self)) { 
        skip_whitespace(self); 
        if (is_at_end(self)) break;

        ResultSigTok result = next_token(self);
        push(&tokens, result.token);
    } 

    Token *eof_token = Token_new(Eof, "", self->line, self->column);
    push(&tokens, *eof_token);

    return tokens;
}

ResultSigTok return_next_token(Token *return_token) {
    ResultSigTok result = {};
    result.token = *return_token;
    result.Error = false;
    return result;
}

ResultSigTok next_token(Lexer *self) {
    unsigned int start_line = self->line;
    unsigned int start_column = self->column;
    char ch = current_char(self);

    switch (ch) {
        case '(': advance(self); return return_next_token(Token_new(LeftParen, "(", start_line, start_column));
        case ')': advance(self); return return_next_token(Token_new(RightParen, ")", start_line, start_column));
        case '{': advance(self); return return_next_token(Token_new(LeftBrace, "{", start_line, start_column));
        case '}': advance(self); return return_next_token(Token_new(RightBrace, "}", start_line, start_column));
        case '[': advance(self); return return_next_token(Token_new(LeftBracket, "[", start_line, start_column));
        case ']': advance(self); return return_next_token(Token_new(RightBracket, "]", start_line, start_column));
        case ';': advance(self); return return_next_token(Token_new(Semicolon, ";", start_line, start_column));
        case ',': advance(self); return return_next_token(Token_new(Comma, ",", start_line, start_column));
        case '.': advance(self); return return_next_token(Token_new(Dot, ".", start_line, start_column));
        case ':': advance(self); return return_next_token(Token_new(Colon, ":", start_line, start_column));
        case '#': advance(self); return return_next_token(Token_new(Hash, "#", start_line, start_column));
        case '+': advance(self); return return_next_token(Token_new(Plus, "+", start_line, start_column));
        case '*': advance(self); return return_next_token(Token_new(Star, "*", start_line, start_column));
        case '/': advance(self); return return_next_token(Token_new(Slash, "/", start_line, start_column));
        case '%': advance(self); return return_next_token(Token_new(Percent, "%", start_line, start_column));
        case '&': advance(self); return return_next_token(Token_new(BitwiseAnd, "&", start_line, start_column));
        case '|': advance(self); return return_next_token(Token_new(BitwiseOr, "|", start_line, start_column));
        case '^': advance(self); return return_next_token(Token_new(BitwiseXor, "^", start_line, start_column));
        case '~': advance(self); return return_next_token(Token_new(BitwiseNot, "~", start_line, start_column));

        case '-':
            advance(self);
            if (current_char(self) == '>') {
                advance(self);
                return return_next_token(Token_new(Arrow, "->", start_line, start_column));
            } else {
                return return_next_token(Token_new(Minus, "-", start_line, start_column));
            }

        case '<':
            advance(self);
            if (current_char(self) == '<') {
                advance(self);
                return return_next_token(Token_new(LeftShift, "<<", start_line, start_column));
            } else if (current_char(self) == '=') {
                advance(self);
                return return_next_token(Token_new(LessEqual, "<=", start_line, start_column));
            } else {
                return return_next_token(Token_new(LessThan, "<", start_line, start_column));
            }

        case '>':
            advance(self);
            if (current_char(self) == '>') {
                advance(self);
                return return_next_token(Token_new(RightShift, ">>", start_line, start_column));
            } else if (current_char(self) == '=') {
                advance(self);
                return return_next_token(Token_new(GreaterEqual, ">=", start_line, start_column));
            } else {
                return return_next_token(Token_new(GreaterThan, ">", start_line, start_column));
            }

        case '=':
            advance(self);
            if (current_char(self) == '=') {
                advance(self);
                return return_next_token(Token_new(Equal, "==", start_line, start_column));
            } else {
                return return_next_token(Token_new(Assign, "=", start_line, start_column));
            }

        case '!':
            advance(self);
            if (current_char(self) == '=') {
                advance(self);
                return return_next_token(Token_new(NotEqual, "!=", start_line, start_column));
            } else {
                return return_next_token(Token_new(Not, "!", start_line, start_column));
            }

        case '\'':
            return read_char_literal(self, start_line, start_column);

        case '"':
            return read_string_literal(self, start_line, start_column);

        default:
            if (isalpha(ch) || ch == '_') {
                return read_identifier_or_keyword(self, start_line, start_column);
            } else if (is_numeric(ch)) {
                return read_number(self, start_line, start_column);
            } else {
                return return_next_token(Token_new(Error, "", start_line, start_column));
            }
    }
}

ResultSigTok read_identifier_or_keyword (Lexer *self, unsigned int start_line, unsigned int start_column) {
    char *identifier = NULL;
    
    while (!is_at_end(self) && (isalnum(current_char(self)) || current_char(self) == '_')) {
        append_char(identifier, current_char(self));
        advance(self);
    }

    // check for type modifers like a<type> or p<type>
    if ((strcmp(identifier, "p") == 0 || strcmp(identifier, "a") == 0) && current_char(self) == '<') {
        char *modifier = identifier;
        advance(self); // eat <
        
        // read the inner type
        char* inner_type = NULL;
        while(!is_at_end(self) && current_char(self) !=  '>') {
            append_char(inner_type, current_char(self));
            advance(self);
        }


        if (current_char(self) == '>') {
            advance(self);
            // outside_single_char<inside_up_to_15>
            // largest type is structinstance
            // so 1+1+15+1 = 18; lets set the size as 20 to be careful
            char full_type[20];
            sprintf(full_type, "%s<%s>", modifier, inner_type);
            
            // assign the type 
            enum TokenType token_type;
            if (strcmp(modifier, "a") == 0) {
                token_type = Array;
            } else {
                token_type = Pointer;
            }
            // token
            Token *return_token = Token_new(token_type, full_type, start_line, start_column);
        
            // result
            ResultSigTok return_result = {};
            return_result.token = *return_token;
            return_result.Error = false;
        
            // return
            return return_result;
        }
    }
    
        
    // check if it is a keyword
    enum TokenType keyword_type = from_keyword(identifier);
    if (keyword_type != Error) {
        Token* return_token = Token_new(keyword_type, identifier, start_line, start_column);
        
        // result
        ResultSigTok return_result = {};
        return_result.token = *return_token;
        return_result.Error = false;
        return return_result;
    }
        
    // token is an error
    Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mIdentifier or token\e[0m" 
    " \e[37m\e[40m Unidentified Identifier or Token\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
    );
    Token *return_token = Token_new(keyword_type, identifier, start_line, start_column);
        
    // result
    ResultSigTok return_result = {};
    return_result.token = *return_token;
    return_result.Error = false;
    return return_result;
}

ResultSigTok read_number (Lexer *self, 
                    unsigned int start_line, 
                    unsigned int start_column) {
    char *number = NULL;
    int is_float = 0;   // bool
    while (!is_at_end(self) && (is_numeric(current_char(self)) || current_char(self) == '.')) {
        if (current_char(self) == '.') {
            if (is_float) {     // when over more than one '.' so the number is like 0.65.7
                    Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40m\e[0m" 
    " \e[37m\e[40m Invalid number format\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
                    );
            }
            is_float = 1;
        }
        
        append_char(number, current_char(self));
        advance(self);
    }

    // check for f at the end of the num (32 bit)
    int is_f32 = 0;
    if (!is_at_end(self) && current_char(self) == 'f') {
        advance(self);
        is_f32 = 1;
    } 
    
    if (is_f32 || is_float) {
        if (is_f32) {
            // f32
            bool works_f32 = check_f32(number, start_line, start_column);
            if (works_f32) {
                // token
                Token* return_token = Token_new(Float32Literal, number, start_line, start_column);
        
                // result
                ResultSigTok return_result = {};
                return_result.token = *return_token;
                return_result.Error = false;
                return return_result;
            } else {
                Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mInvalid float32 literal\e[0m" 
    " \e[37m\e[40m\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, 
        start_column
                    );
            }
        } else {
            // f64
            bool works_f64 = check_f64(number, start_line, start_column);
            if (works_f64) {
                // token
                Token *return_token = Token_new(FloatLiteral, number, start_line, start_column);
        
                // result
                ResultSigTok return_result = {};
                return_result.token = *return_token;
                return_result.Error = false;
                return return_result;
            } else {
                Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mInvalid float32 literal\e[0m" 
    " \e[37m\e[40m\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, 
        start_column
                    );
            }
        }
    } else {
        // token
        Token *return_token = Token_new(IntLiteral, number, start_line, start_column);
        
        // result
        ResultSigTok return_result = {};
        return_result.token = *return_token;
        return_result.Error = false;
        return return_result;
    }
    // most likely an error happened so lets return Error
    // token
    Token *return_token = Token_new(Error, "", start_line, start_column);
        
    // result
    ResultSigTok return_result = {};
    return_result.token = *return_token;
    return_result.Error = true;
    return return_result;
}

ResultSigTok read_char_literal (Lexer* self, 
                            unsigned int start_line,
                            unsigned int start_column) {
    advance(self); // consume opening '

    if (is_at_end(self)) {
        Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mchar literal,\e[0m" 
    " \e[37m\e[40m Unterminated char literal\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, 
        start_column
        );
    }
    
    char ch;
    if (current_char(self) == '\\') {
        advance(self);
        
        if (is_at_end(self)) {
            Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    "\e[4m\e[36m\e[40mchar literal, \e[0m" 
    "\e[37m\e[40mUnterminated char literal\e[0m" 
    "\n\t\e[0m" 
    "\e[4m At line\e[0m" 
    "\e[1m\e[33m\e[40m %d\e[0m" 
    "\e[4m , and Column\e[0m" 
    "\e[1m\e[33m\e[40m %d\e[0m\n",
        start_line, 
        start_column
            );
        }
        
        switch (current_char(self)) {
            case 'n' : ch = '\n'; break;
            case 't' : ch = '\t'; break;
            case '0' : ch = '\0'; break;
            case '\\' : ch = '\\'; break;
            case '\'' : ch = '\''; break;
            
            case '[' :  // handle /[num] syntax
                advance(self);
                if (is_at_end(self) || !is_numeric(current_char(self))) {
                    Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    "\e[4m\e[36m\e[40m\e[0m" 
    "\e[37m\e[40m Invalid escape sequence\e[0m" 
    "\n\t\e[0m" 
    "\e[4m At line\e[0m" 
    "\e[1m\e[33m\e[40m %d\e[0m" 
    "\e[4m , and Column\e[0m" 
    "\e[1m\e[33m\e[40m %d\e[0m\n",
        start_line, start_column
                    );
                }
                
                
                int num = to_digit(current_char(self), 10);
                if (is_at_end(self) || current_char(self) != ']') {
                    Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mchar literal,\e[0m" 
    " \e[37m\e[40m Expected ']' in escape sequence\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
                    );
                }
                ch = (char)num;
                break;
            default : ch = current_char(self); break;
        }
    } else {
        ch = current_char(self);
    }
        
        
    advance(self);
    
    if (is_at_end(self) || current_char(self) != '\'') {
        Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mChar Literal\e[0m" 
    " \e[37m\e[40m Unterminated char literal\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
    start_line, start_column
        );
    }
    
    advance(self); // consume closing '
    
    char *string = NULL;
    append_char(string, ch);
    
    // token
    Token *return_token = Token_new(CharLiteral, string, start_line, start_column);
        
    // result
    ResultSigTok return_result = {};
    return_result.token = *return_token;
    return_result.Error = false;
    return return_result;
}


ResultSigTok read_string_literal (Lexer *self, 
                            unsigned int start_line, 
                            unsigned int start_column) {
    advance(self);
    
    char *string = NULL;
    
    while (!is_at_end(self) && current_char(self) != '"') {
        if (current_char(self) == '\\') {
            advance(self);
            
            if (is_at_end(self)) {
                Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mString Literal\e[0m" 
    " \e[37m\e[40m Unterminated string literal\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
                );
            }
            
                
            char escaped;
            switch (current_char(self)) {
                case 'n' : escaped = '\n'; break;
                case 't' : escaped = '\t'; break;
                case '0' : escaped = '\0'; break;
                case '\\': escaped = '\\'; break;
                case '"' : escaped = '"'; break;
                default : escaped = current_char(self); break;
            } 
            
            append_char(string, escaped);
        } else {
            append_char(string, current_char(self));
        }
        advance(self);
        
        if (is_at_end(self)) {
            Err (
    "\e[31m\e[1m\e[4m\e[40mLEXER ERROR - \e[0m" 
    " \e[4m\e[36m\e[40mString literal,\e[0m" 
    " \e[37m\e[40m Unterminated string literal\e[0m" 
    "\n\t\e[0m" 
    "\e[4mAt line \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m" 
    "\e[4m , and Column \e[0m" 
    "\e[1m\e[33m\e[40m%d\e[0m\n",
        start_line, start_column
            );

        }

        advance(self); // consume closing "
    }
    
    
    // token
    Token *return_token = Token_new(StringLiteral, string, start_line, start_column);
        
    // result
    ResultSigTok return_result = {};
    return_result.token = *return_token;
    return_result.Error = false;
    return return_result;
}


void skip_whitespace (Lexer *self) {
    while (!is_at_end(self)) {
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

char current_char(Lexer *self) {
    if (is_at_end(self)) {
        return '\0';
    } else {
        return self->input[self->position];
    }
}

void advance (Lexer *self) {
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

bool is_at_end (Lexer *self) {
    return (self->position >= strlen(self->input)); 
}

char *append_char_impl(char *str, char c) {
    size_t len = 0;

    if (str != NULL) {
        len = strlen(str);
    }

    // allocate or expand
    char *new_str = realloc(str, len + 2);
    if (!new_str) {
        free(str);
        fprintf(stderr, "append_char_impl: memory allocation failed\n");
        exit(EXIT_FAILURE);
     }

    new_str[len] = c;
    new_str[len + 1] = '\0';
    return new_str;
}


bool is_numeric(char s) {
    switch (s) {
        case '1': return true;
        case '2': return true;
        case '3': return true;
        case '4': return true;
        case '5': return true;
        case '6': return true;
        case '7': return true;
        case '8': return true;
        case '9': return true;
        case '0': return true;
        default: return false;
    }
}

int to_digit(char c, int radix) {
    if (radix < 2 || radix > 36) {
        // radix out of supported range (0-9, A-Z/a-z)
        return -1; 
    }

    if (isdigit(c)) {
        int digit_value = c - '0';
        if (digit_value < radix) {
            return digit_value;
        }
    } else if (isalpha(c)) {
        int digit_value;
        if (islower(c)) {
            digit_value = c - 'a' + 10;
        } else { // isupper(c)
            digit_value = c - 'A' + 10;
        }
        
        if (digit_value < radix) {
            return digit_value;
        }
    }
    
    return -1; // not a valid digit for the given radix
}
