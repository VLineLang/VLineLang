#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <vector>
#include <string>
#include <variant>
#include "../vm/bignum.hpp"

enum BytecodeOp {
    LOAD_CONST,        // 加载常量值（数值/字符串）
    LOAD_VAR,          // 加载变量值
    STORE_VAR,         // 存储到变量
    BINARY_OP,         // 二元运算（含算术和比较）
    JUMP_IF_FALSE,     // 条件跳转（检测栈顶值）
    CALL_FUNCTION,     // 函数调用
    JUMP,              // 无条件跳转（绝对地址）
    RETURN,            // 函数返回
    BUILD_LIST,        // 构建列表
    POP,               // 弹出栈顶元素
    LOAD_SUBSCRIPT,    // 加载列表元素
    STORE_SUBSCRIPT,   // 存储到列表元素
    CREATE_OBJECT,     // 创建对象
    LOAD_MEMBER,       // 加载对象成员
    STORE_MEMBER,      // 存储到对象成员
    LOAD_FUNC,         // 加载函数闭包
    STORE_MEMBER_FUNC, // 存储成员函数到对象
    CLEAR,             // 清空栈
    LABEL              // 标签（用于跳转目标）
};

struct CallFunctionOperand {
    std::string funcName;
    int argCount;
};

struct Bytecode {
    BytecodeOp op;
    std::variant<BigNum, std::string, CallFunctionOperand> operand;
};

using BytecodeProgram = std::vector<Bytecode>;

#endif