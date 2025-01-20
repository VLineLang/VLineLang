<div align="center">

# ⚡ VLineLang
A modern, clean, fast and powerful high-level programming language.

![Last Commit](https://img.shields.io/github/last-commit/VLineLang/VLineLang)
![License](https://img.shields.io/github/license/VLineLang/VLineLang)
![Stars](https://img.shields.io/github/stars/VLineLang/VLineLang)
![Forks](https://img.shields.io/github/forks/VLineLang/VLineLang)
![Commit](https://img.shields.io/github/commit-activity/m/VLineLang/VLineLang)

[>> Chinese README >>](README_zh.md)

</div>

**The project is currently under development. We look forward to more PRs and Issue submissions. Thanks to all the developers who have contributed to the project! :)**

## 🚀 Features

### Functions

- **Member Functions**:
    - `list.size()`: Returns the length of the list.
    - `list.empty()`: Checks if the list is empty.
    - `list.append(value)`: Appends an element to the end of the list.
    - `list.insert(index, value)`: Inserts an element at the specified position.
    - `list.erase(begin, end)`: Deletes elements within the specified range in the list.

- **Built-in Functions**:
    - `print(value)`: Prints a value to the console.
    - `input(prompt)`: Reads a value from user input.
    - `len(value)`: Returns the length of a string or list.
    - `type(value)`: Returns the type of a value.
    - `range(start, end)`: Generates a list of numbers.
    - `time()`: Returns the current timestamp.
    - `sleep(ms)`: Pauses execution for the specified number of milliseconds.
    - `system(command)`: Executes a system command.
    - `exit(code)`: Exits the program and returns the specified status code.
    - `read(filename)`: Reads the contents of a file.
    - `write(filename, content)`: Writes content to a file.
    - `import(filename)`: Imports and executes an external file.

### Keywords

- **Control Structures**:
    - `if`: Conditional judgment.
    - `else`: The negative branch of a conditional judgment.
    - `while`: Loop structure.
    - `break`: Breaks out of a loop.
    - `continue`: Skips the current loop iteration.

- **Function Definitions**:
    - `func`: Defines a function.
    - `return`: Returns a value from a function.

- **Special Values**:
    - `null`: Represents a null value.
    - `true`: Represents a true value.
    - `false`: Represents a false value.

- **Logical Operations**:
    - `and`: Logical AND.
    - `or`: Logical OR.
    - `not`: Logical NOT.

### Features

- **Variables and Data Types**:
    - Supports dynamic type inference; no need to explicitly declare variable types.
    - Supports multiple data types, including:
        - Numbers (`Number`): Integers and floating-point numbers.
        - Strings (`String`): Text data.
        - Lists (`List`): Ordered collections.
        - Null values (`null`): Represents no value.

- **Control Structures**:
    - Supports conditional judgments (`if`, `else`).
    - Supports loops (`while`).
    - Supports breaking out of loops (`break`, `continue`).

- **Functions**:
    - Supports function definition and invocation.
    - Supports recursion and nested functions.
    - Supports function parameters and return values.

- **Modularity**:
    - Supports importing external files via `import`.
    - Supports modular programming.

- **Recursion Depth Control**:
    - Controls recursion depth via the `--depth` parameter to prevent stack overflow.

## 🛠️ Quick Start

### Installation

**Building from Source**

VLineLang is currently in the development stage. You can run it by following these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/VLineLang/VLineLang.git
   cd VLineLang
   ```

2. Compile the project:
   ```bash
   g++ -std=c++17 -o vline_lang main.cpp
   ```

3. Run the compiler:
   ```bash
   ./vline_lang
   ```

**You can also get the latest version from the Release page.**

### Running

You can run VLineLang code in the following ways:

1. **Interactive Mode**:
   ```bash
   ./vline_lang
   ```
   In interactive mode, you can input code line by line and see the results immediately.

2. **Script Mode**:
   ```bash
   ./vline_lang script.vl
   ```
   By specifying a script file, VLineLang will execute the code in the file.

3. **Command Line Arguments**:
    - `--in <filename>`: Reads input from the specified file.
    - `--out <filename>`: Redirects output to the specified file.
    - `--depth <value>`: Sets the recursion depth limit.

## 📝 Language Syntax

### Variable Declaration

VLineLang allows direct variable declaration and initialization, supporting dynamic type inference:
```vl
x = 10
name = "VLineLang"
list = [1, 2, 3]
```

### Control Structures

VLineLang supports common control structures such as `if` and `while` loops:
```vl
if x > 10 {
    print("x is greater than 10")
} else {
    print("x is less than or equal to 10")
}

while x > 0 {
    print(x)
    x = x - 1
}
```

### Functions

VLineLang supports function definition and invocation. Functions can return values and be called recursively:
```vl
func fact(n) {
    if n <= 1 {
        return 1
    }
    return n * fact(n - 1)
}

print(fact(5))  // Outputs 120
```

## TODO List

- [ ] for loops
- [ ] classes
- [ ] exception handling
- [ ] standard library support
- [ ] performance optimization
- [ ] documentation improvement
- [ ] C/C++ embedding

## 🤝 Contribution Guidelines

We welcome all developers to contribute to VLineLang! If you are interested in contributing code or making suggestions, please follow these steps:

1. **Fork the Repository**: Fork this project on GitHub.
2. **Create a Branch**: Create a new branch for your feature or fix.
3. **Make Changes**: Make changes on the branch and commit them.
4. **Submit a PR**: Submit a Pull Request to the main repository, describing your changes.

**Please ensure your code adheres to the project's coding standards and passes all tests.**

## 🙏 Acknowledgments

- [@karsl-program](https://github.com/karsl-program/) (Project Lead, Main Developer)
- [@cleversheep2011](https://github.com/cleversheep2011/) (Developer, Team Member, Project Advisor)