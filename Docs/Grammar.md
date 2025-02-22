```ebnf
<basic_type> ::= "u8"
               | "u16"
               | "u32"
               | "u64"
               | "i8"
               | "i16"
               | "i32"
               | "i64"
               | "char"
               | "f32"
               | "f64"

<all_type> ::= <basic_type>

<type> ::= "const" <all_type>
         | <all_type> "*"

# Match any number of whitespaces
<whitespace> ::= {" " | "\t" | "\b" | "\r" | "\n" | "\f"}

<program> ::= <whitespace> <type> <program> | <whitespace> <type>
```
