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
- C language type system that can parse declarations like: 
'''int *func[2][3](int a, int b);
'''TODO:
