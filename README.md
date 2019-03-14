# LilCompiler

## Environment
This project requires three cmds(flex, bison, g++) and one dynamic library(libfl).

## Installation
Run 'make' to get the executable file.

## Features
- Macros: 
    - #include
    - #define, #undef
- generates ASTs for .c files.
- a parser conforms to the BNF of C language.
- supports all C expression precedences.
- a type system that can parse declarations like: 
```
int *func[2][3](int *i, int j);
```
, and a expression type checker.
- TODO: intermediate code generation.
