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

**The project is currently under development. We look forward to more PRs and Issue submissions. Thanks to all the developers who have contributed to the project! :)**

## 🚀 Features

### Functions

- **Member Functions**:
    - `list.append(value)`: Appends an element to the end of the list.
    - `list.insert(index, value)`: Inserts an element at the specified position.
    - `list.erase(begin, end)`: Removes elements within the specified range from the list.

- **Built-in Functions**:
    - `number(value)`: Converts a value to the number type.
    - `str(value)`: Converts a value to the string type.
    - `list(value)`: Converts a value to the list type.
    - `print(value)`: Prints a value to the console.
    - `input(prompt)`: Reads a value from user input.
    - `len(value)`: Returns the length of a string or list.
    - `type(value)`: Returns the type of a value.
    - `range(start, end)`: Generates a list of numbers.
    - `time()`: Returns the current timestamp.
    - `sleep(ms)`: Pauses execution for the specified number of milliseconds.
    - `system(command)`: Executes a system command.
    - `exit(code)`: Exits the program with the specified status code.
    - `read(filename)`: Reads the contents of a file.
    - `write(filename, content)`: Writes content to a file.
    - `append(list, value)`: Appends an element to the end of the list.
    - `insert(list, index, value)`: Inserts an element at the specified position.
    - `erase(list, begin, end)`: Removes elements within the specified range from the list.

### Keywords

- **Control Structures**:
    - `if`: Conditional statement.
    - `else`: Negation branch of a conditional statement.
    - `while`: Loop structure.
    - `for`: Loop structure.
    - `break`: Breaks out of a loop.
    - `continue`: Skips the current iteration of a loop.

- **Function Definitions**:
    - `fn`: Defines a function.
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
        - Null (`null`): Represents no value.
        - Objects (`Object`): Classes.

- **Control Structures**:
    - Supports conditional statements (`if`, `else`).
    - Supports loops (`while`, `for`).
    - Supports breaking out of loops (`break`, `continue`).

- **Functions**:
    - Supports function definition and invocation.
    - Supports recursion and nested functions.
    - Supports function parameters and return values.

- **Object-Oriented (Class) Support**:
    - Supports class definition and instantiation.
    - Supports class member variables and member functions.
    - Supports inheritance and polymorphism.

- **Big Number Support**:
    - The default built-in type `number` supports big number operations.
    - Supports arithmetic operations (including modulo) and comparison operations.
    - Integer precision is infinite; floating-point precision is up to 20 decimal places (i.e., `1e-20`).
    - Fast performance with constant-time operations.

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
   cmake .
   ```

3. Run the compiler:
   ```bash
   ./vline_lang
   ```

**You can also download the latest version from the Release page.**

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

3. **Command-Line Arguments**:
    - `--in <filename>`: Reads input from the specified file.
    - `--out <filename>`: Redirects output to the specified file.

## 📝 Language Syntax

### Variable Declaration

VLineLang allows direct variable declaration and initialization, supporting dynamic type inference:
```vl
x = 10
name = "VLineLang"
list = [1, 2, 3]
```

### Control Structures

VLineLang supports common control structures such as `if`, `while` loops, and `for` loops:
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

for i in range(0, 10) {
    print(i)
}
```

### Functions

VLineLang supports function definition and invocation, with support for return values and recursion:
```vl
fn fact(n) {
    if n <= 1 {
        return 1
    }
    return n * fact(n - 1)
}

print(fact(5))  // Outputs 120
```

### Class Support

VLineLang supports class definition and instantiation, with support for inheritance and polymorphism:
```vl
class Person {
    fn __init__(name, age) {
        self.name = name
        self.age = age
    }
    fn say_hello() {
        print("Hello, my name is ", self.name, " and I am ", self.age, " years old.\n")
    }
}

class Student : Person {
    fn __init__(name, age, grade) {
        self.name = name
        self.age = age
        self.grade = grade
    }
    fn say_hello() {
        print("Hello, I am a student. My name is ", self.name, " and I am ", self.age, " years old. My grade is ", self.grade, ".\n")
    }
}

people = new Person("Alice", 20)
student = new Student("Bob", 18, 7)

people.say_hello()
student.say_hello()
```

## TODO List

- [ ] Exception Handling
- [ ] Performance Optimization
- [ ] Documentation Improvement
- [ ] C/C++ Embedding

## 🤝 Contribution Guidelines

We welcome all developers to contribute to VLineLang! If you are interested in contributing code or suggestions, please follow these steps:

1. **Fork the Repository**: Fork the project on GitHub.
2. **Create a Branch**: Create a new branch for your feature or fix.
3. **Commit Changes**: Make changes and commit them in your branch.
4. **Submit a PR**: Submit a Pull Request to the main repository, describing your changes.

**Please ensure your code adheres to the project's coding standards and passes all tests.**

## 🙏 Acknowledgments

- [@karsl-program](https://github.com/karsl-program/) (Project Lead, Main Developer)
- [@cleversheep2011](https://github.com/cleversheep2011/) (Developer, Team Member, Project Advisor)