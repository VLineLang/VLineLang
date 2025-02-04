<div align="center">

# ⚡ VLineLang
一个现代化的、简洁的、快速的、强大的高级编程语言。

![Last Commit](https://img.shields.io/github/last-commit/VLineLang/VLineLang)
![License](https://img.shields.io/github/license/VLineLang/VLineLang)
![Stars](https://img.shields.io/github/stars/VLineLang/VLineLang)
![Forks](https://img.shields.io/github/forks/VLineLang/VLineLang)
![Commit](https://img.shields.io/github/commit-activity/m/VLineLang/VLineLang)

[>> 英文文档 >>](README.md)

</div>

**目前项目正处于开发阶段，期待更多的 PR 和 Issue 提交，感谢所有为项目贡献的开发者！:)**

## 🚀 功能特性

### 函数

- **成员函数**：
    - `list.append(value)`：向列表末尾添加一个元素。
    - `list.insert(index, value)`：在指定位置插入一个元素。
    - `list.erase(begin, end)`：删除列表中指定范围的元素。

- **内置函数**：
    - `print(value)`：打印值到控制台。
    - `input(prompt)`：从用户输入读取值。
    - `len(value)`：返回字符串或列表的长度。
    - `type(value)`：返回值的类型。
    - `range(start, end)`：生成一个数字列表。
    - `time()`：返回当前时间戳。
    - `sleep(ms)`：暂停执行指定的毫秒数。
    - `system(command)`：执行系统命令。
    - `exit(code)`：退出程序并返回指定的状态码。
    - `read(filename)`：读取文件内容。
    - `write(filename, content)`：将内容写入文件。
    - `append(list, value)`：向列表末尾添加一个元素。
    - `insert(list, index, value)`：在指定位置插入一个元素。
    - `erase(list, begin, end)`：删除列表中指定范围的元素。

### 关键字

- **控制结构**：
    - `if`：条件判断。
    - `else`：条件判断的否定分支。
    - `while`：循环结构。
    - `for`：循环结构。
    - `break`：跳出循环。
    - `continue`：跳过当前循环迭代。

- **函数定义**：
    - `fn`：定义函数。
    - `return`：从函数返回值。

- **特殊值**：
    - `null`：表示空值。
    - `true`：表示真值。
    - `false`：表示假值。

- **逻辑操作**：
    - `and`：逻辑与。
    - `or`：逻辑或。
    - `not`：逻辑非。

### 功能

- **变量与数据类型**：
    - 支持动态类型推断，无需显式声明变量类型。
    - 支持多种数据类型，包括：
        - 数字（`Number`）：整数和浮点数。
        - 字符串（`String`）：文本数据。
        - 列表（`List`）：有序集合。
        - 空值（`null`）：表示无值。
        - 对象（`Object`）：类。

- **控制结构**：
    - 支持条件判断（`if`、`else`）。
    - 支持循环（`while`、`for`）。
    - 支持跳出循环（`break`、`continue`）。

- **函数**：
    - 支持函数定义和调用。
    - 支持递归和嵌套函数。
    - 支持函数参数和返回值。

- **面向对象（类）的支持**：
    - 支持类的定义和实例化。
    - 支持类的成员变量和成员函数。
    - 支持继承和多态。

- **大数支持**：
    - 默认内置类型 `number` 支持大数运算。
    - 支持四则运算（包括取模）和比较操作。
    - 整型无穷精度，浮点数精度为小数点后20位（即 `1e-20`）。
    - 速度快，常数级别运算。

## 🛠️ 快速开始

### 安装

**从源代码构建**

VLineLang 目前处于开发阶段，您可以通过以下步骤运行：

1. 克隆仓库：
   ```bash
   git clone https://github.com/VLineLang/VLineLang.git
   cd VLineLang
   ```

2. 编译项目：
   ```bash
   cmake .
   ```

3. 运行编译器：
   ```bash
   ./vline_lang
   ```

**也可以从Release页获取最新版本。**

### 运行

您可以通过以下方式运行 VLineLang 代码：

1. **交互式模式**：
   ```bash
   ./vline_lang
   ```
   在交互式模式下，您可以逐行输入代码并立即看到执行结果。

2. **脚本模式**：
   ```bash
   ./vline_lang script.vl
   ```
   通过指定脚本文件，VLineLang 将执行文件中的代码。

3. **命令行参数**：
    - `--in <filename>`：从指定文件读取输入。
    - `--out <filename>`：将输出重定向到指定文件。


## 📝 语言语法

### 变量声明

VLineLang 可直接声明变量并初始化，支持动态类型推断：
```vl
x = 10
name = "VLineLang"
list = [1, 2, 3]
```

### 控制结构

VLineLang 支持常见的控制结构，如 `if`、`while` 循环、`for` 循环：
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

### 函数

VLineLang 支持函数定义和调用，函数可以返回值和递归调用：
```vl
fn fact(n) {
    if n <= 1 {
        return 1
    }
    return n * fact(n - 1)
}

print(fact(5))  // 输出 120
```

### 类的支持

VLineLang 支持类的定义和实例化，支持继承和多态：
```vl
class Person {
    fn __init__(name, age) {
        self.name = name
        self.age = age
    }
    fn say_hello() {
        print("Hello, my name is ", self.name, " and I am ", self.age, " years old.")
    }
}

class Student : Person {
    fn __init__(name, age, grade) {
        self.name = name
        self.age = age
        self.grade = grade
    }
    fn say_hello() {
        print("Hello, I am a student. My name is ", self.name, " and I am ", self.age, " years old. My grade is ", self.grade, ".")
    }
}

people = Person("Alice", 20)
student = Student("Bob", 18, 7)

people.say_hello()
student.say_hello()
```

## TODO List

- [ ] 异常处理
- [ ] 标准库支持
- [ ] 性能优化
- [ ] 文档完善
- [ ] C/C++ 内嵌


## 🤝 贡献指南

我们欢迎所有开发者参与 VLineLang 的开发！如果您有兴趣贡献代码或提出建议，请遵循以下步骤：

1. **Fork 仓库**：在 GitHub 上 Fork 本项目。
2. **创建分支**：为您的功能或修复创建一个新的分支。
3. **提交更改**：在分支上进行更改并提交。
4. **提交 PR**：向主仓库提交 Pull Request，描述您的更改。

**请确保您的代码符合项目的编码规范，并通过所有测试。**


## 🙏 鸣谢

- [@karsl-program](https://github.com/karsl-program/) (项目负责人、主要开发者)
- [@cleversheep2011](https://github.com/cleversheep2011/) (开发者、团队成员、项目建议提供者)
