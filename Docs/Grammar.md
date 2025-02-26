# How To Read

I've tried to follow something similar to an EBNF (Extended-Backus-Naur-Form) syntax.
This is not absolute though, because I sometimes use regex expressions to simplify things,
because when reading a large grammar (like that of a programming language), following
rules one after another can be annoying, just to realize in the end that the actual language
the subset of productions generate is a regular language (something that has a regular expression).

## Syntax

- Production rules are always in the form `"<" RULE_NAME ">"`.
- `(...)` means the thing inside parenthesis is repeated atleast once.
- `{...}` means the thing inside the braces is repeated any number of times (zero or more).
- `[...]` means the thing inside the brackets can be repeated at max once (zero or once).
- If something is inside `".."` (double quotes), then it's a terminal symbol. If it's not inside `".."`,
  then it's 
- Alternations are piped, and concatenations are just written one after another.
  - `<R> ::= "a" | "b"` is a rule that can alternatively select between terminal `a` and `b`
  - `<M> ::= "a" <X>` is a rule that first reads terminal `a` and then applies rule `<X>`.
- Regular expressions are not inside `".."`, but they are terminals.

# The Grammar

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

<whitespace> ::= {" " | "\t" | "\b" | "\r" | "\n" | "\f"}

<ascii_string> ::= '"' {*} '"'
<ascii_char> ::= "'" {*} "'"

<int> ::= (0-9) 
<flt> ::= <int> "." [<int>] ["f"]
        | <int>
<hex> ::= (0-9A-Fa-f)
<bin> ::= ("0"| "1" | "_") ["b"]

<num> ::= <int> 
        | <flt>
        | <hex>
        | <bin>

<expr_list> ::= <expr0> "," <expr_list> 
              | <expr0>

<expr> ::= <expr_list>
         | <num>
         | <id>
         | <ascii_string> 
         | <ascii_char>

<expr0> ::= <expr1> "=" <expr0> # To allow mutliple assignments at the same time
          | <expr1> "+=" <expr1> # Then following this, multiple assignments are disabled
          | <expr1> "-=" <expr1>
          | <expr1> "*=" <expr1>
          | <expr1> "/=" <expr1>
          | <expr1> "%=" <expr1>
          | <expr1> ">>=" <expr1>
          | <expr1> "<<=" <expr1>
          | <expr1> "&=" <expr1>
          | <expr1> "|=" <expr1>
          | <expr1> "^=" <expr1>
          | <expr1>

<expr1> ::= <expr2> "?" <expr1> ":" <expr1>
          | <expr2>

<expr2> ::= <expr3> "||" <expr2>
          | <expr3>

<expr3> ::= <expr4> "&&" <expr3>
          | <expr4>

<expr4> ::= <expr5> "|" <expr4>
          | <expr5>

<expr5> ::= <expr6> "^" <expr5>
          | <expr6>

<expr6> ::= <expr7> "&" <expr6>
          | <expr7>

<expr7> ::= <expr8> "==" <expr7>
          | <expr8> "!=" <expr7>
          | <expr8>

<expr8> ::= <expr9> "<" <expr8>
          | <expr9> ">" <expr8>
          | <expr9> "<=" <expr8>
          | <expr9> ">=" <expr8>
          | <expr9>

<expr9> ::= <expr10> "<<" <expr9>
          | <expr10> ">>" <expr9>
          | <expr10>

<expr10> ::= <expr11> "+" <expr10>
           | <expr11> "-" <expr10>
           | <expr11>
          
<expr11> ::= <expr12> "*" <expr11>
           | <expr12> "/" <expr11>
           | <expr12> "%" <expr11>
           | <expr12>

<expr12> ::= "++" <expr12>
           | "--" <expr12>
           | "+" <expr12>
           | "-" <expr12>
           | "!" <expr12>
           | "~" <expr12>
           | "(" <type> ")"  <expr12>
           | "*" <expr12>
           | "&" <expr12>
           | "sizeof" <expr12>
           | "_Alignof" <expr12>
           | <expr13>

<expr13> ::= <expr14> "++" 
           | <expr14> "--" 
           | <expr14> "(" <expr14> ")" 
           | <expr14> "[" <expr14> "]" 
           | <expr14> "." <expr14> 
           | <expr14> "->" <expr14>
           | "(" <type> ")" "{" <expr_list> "}"
           | <expr14>

<expr14> ::= "(" <expr14> ")" 
           | <expr>

<id> ::= [_]{_a-zA-Z0-9}

<var_decl> ::= <type> <id>

<assign_stmt> ::= <type> <id> "=" <expr>

<program> ::= <whitespace> <type> <program> | <whitespace> <type>
```
