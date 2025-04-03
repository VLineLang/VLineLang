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

## 特性

- **基本数据类型**
  - 数字（支持小数）
  - 字符串（支持转义序列）
  - 列表
  - 对象

- **控制结构**
  - if-else 条件语句
  - while 循环
  - for 循环
  - break 和 continue 语句

- **函数**
  - 函数声明
  - return 语句
  - 内置函数

- **面向对象编程**
  - 类声明
  - 继承
  - 成员函数
  - 对象属性

- **常量**
  - 常量声明

- **运算符**
  - 算术运算：+, -, *, /, %, ^
  - 比较运算：<, <=, ==, !=, >=, >
  - 列表索引：[]

## 内置函数

- 输入/输出：`print`, `input`
- 列表操作：`len`, `append`, `erase`, `insert`
- 类型操作：`type`, `list`, `str`, `number`
- 数学函数：`floor`, `ceil`, `abs`, `pow`, `round`, `sqrt`
- 系统函数：`sleep`, `system`, `exit`, `time`
- 文件操作：`read`, `write`

## 项目结构

- `lexer/`：源代码词法分析
  - `lexer.hpp`：词法分析器
  - `token.hpp`：词法单元定义

- `parser/`：语法分析
  - `parser.hpp`：解析器实现
  - `errors.hpp`：错误处理
  - `value.hpp`：值表示

- `ast/`：抽象语法树
  - `ast.hpp`：AST节点定义

- `vm/`：虚拟机
  - `vm.hpp`：虚拟机实现
  - `bignum.hpp`：大数运算

- `std/`：标准库
  - 内置函数和工具

## 文档

详细的语言文档和使用示例，请访问我们的[官方文档](https://vlinelang.github.io/)。

## 许可证

本项目基于仓库中LICENSE文件的条款进行许可。