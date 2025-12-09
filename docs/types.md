## Variables, Literals, Operations & Types

## Variable definition
let name: type = value;
let name: type;

## Types
| type | size | definition syntax |
| --- | --- | --- |
| i16 | 16 bit signed integer |
| i32 | 32 bit signed integer |
| i64 | 64 bit signed integer |
| u16 | 16 bit unsigned integer |
| u32 | 32 bit unsigned integer |
| u64 | 64 bit unsigned integer |
| f32 | signed 32 bit floating point number |
| f64 | signed 64 bit floating point number |
| char | normal c like char (ASCII table only) |
| structinstance | makes a struct | `let name: struct = new StructName;` |
| defstruct | defining a struct | `let Struct : defstruct = { let num1 : i32, let num2 : u64 };` |
| a<type> | an array with the type inside of type indexes start at 0. *Fixed size* | `let arrays : a<f64> = {54.76, 6.54};` so size is 2 |
| p<type> | a pointer to the type, works exactly like c pointers. *can reallocate size later*| `let pointer : p<char> = "Hello, world";` |



## literals
*`*` means same as syntax*
| type | syntax | preprosessor infers as |
|---|---|---|
| array | `{value, value, value}` | * |
| string | "this" | {'t', 'h', 'i', 's', '\0'} |
| Character | *ASCII only `'a'` or `'b'` `'\n'` `'\t'` `'\0'` | * |
| Ascii char escape | `'\[65]'` | will be the 4th character (decimal) on the ascii table or `'A'` |
| Integer | `43` or `5` or `7`| * |
| Floating point (32 bit) | `65.64f` | `65.64` |
| Floating point (64 bit) | `65.64`  | `65.64` |

# Operations
`*` *all operations start from inside of the parenthesis and move outwards left to right following PEMDAS (american)*
| syntax | operation | 
|---|---|
| `*` | multiplied |
| `+` | addition |
| `-` | subtraction |
| `/` | divided by |
| `>>` | bitshift right |
| `<<` | bitshift left |
| `\|` | or |
| `&` | and |
| `^` | xor |
| `%` | mod |
|  `~` | bitwise not |


### structs
```rust
let Struct : defstruct = {
    let num1 : i32,
    let num2 : u64,
};

let instance : structinstance = new Struct;     // "new" means allocate on the heap
instance.num1 = 5;
instance.num2 = 7;
```


