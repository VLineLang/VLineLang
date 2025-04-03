<div align="center">

# ⚡ VLineLang
A modern, concise, fast, and powerful high-level programming language.

![Last Commit](https://img.shields.io/github/last-commit/VLineLang/VLineLang)
![License](https://img.shields.io/github/license/VLineLang/VLineLang)
![Stars](https://img.shields.io/github/stars/VLineLang/VLineLang)
![Forks](https://img.shields.io/github/forks/VLineLang/VLineLang)
![Commit](https://img.shields.io/github/commit-activity/m/VLineLang/VLineLang)

[>> Chinese Documentation >>](README_zh.md)

</div>

## Features

- **Basic Data Types**
  - Numbers (with decimal support)
  - Strings (with escape sequences)
  - Lists
  - Objects

- **Control Structures**
  - If-else statements
  - While loops
  - For loops
  - Break and continue statements

- **Functions**
  - Function declarations
  - Return statements
  - Built-in functions

- **Object-Oriented Programming**
  - Class declarations
  - Inheritance
  - Member functions
  - Object properties

- **Constants**
  - Constant declarations

- **Operators**
  - Arithmetic: +, -, *, /, %, ^
  - Comparison: <, <=, ==, !=, >=, >
  - List indexing: []

## Built-in Functions

- Input/Output: `print`, `input`
- List Operations: `len`, `append`, `erase`, `insert`
- Type Operations: `type`, `list`, `str`, `number`
- Math Functions: `floor`, `ceil`, `abs`, `pow`, `round`, `sqrt`
- System Functions: `sleep`, `system`, `exit`, `time`
- File Operations: `read`, `write`

## Project Structure

- `lexer/`: Tokenization of source code
  - `lexer.hpp`: Lexical analyzer
  - `token.hpp`: Token definitions

- `parser/`: Syntax analysis
  - `parser.hpp`: Parser implementation
  - `errors.hpp`: Error handling
  - `value.hpp`: Value representations

- `ast/`: Abstract Syntax Tree
  - `ast.hpp`: AST node definitions

- `vm/`: Virtual Machine
  - `vm.hpp`: VM implementation
  - `bignum.hpp`: Big number arithmetic

- `std/`: Standard Library
  - Built-in functions and utilities

## Documentation

For detailed language documentation and usage examples, please visit our [official documentation](https://vlinelang.github.io/).

## License

This project is licensed under the terms of the LICENSE file included in the repository.