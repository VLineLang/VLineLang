#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <vector>
#include <string>
#include <variant>
#include "../vm/bignum.hpp"

enum BytecodeOp {
    LOAD_CONST,    // 加载常量值（数值/字符串）
    LOAD_VAR,      // 加载变量值
    STORE_VAR,     // 存储到变量
    BINARY_OP,     // 二元运算（含算术和比较）
    JUMP_IF_FALSE, // 条件跳转（检测栈顶值）
    CALL_FUNCTION, // 函数调用
    JUMP,          // 无条件跳转（绝对地址）
    RETURN,        // 函数返回
    BUILD_LIST,    // 构建列表
    GET_ITER,      // 获取迭代器
    FOR_ITER,       // 迭代控制
    POP,           // 弹出栈顶元素
    LOAD_SUBSCRIPT,   // 加载列表元素
    STORE_SUBSCRIPT,   // 存储到列表元素
    CREATE_OBJECT,
    LOAD_MEMBER,
    STORE_MEMBER
};

enum CompareOp {
    CMP_LT,
    CMP_LE,
    CMP_EQ,
    CMP_NE,
    CMP_GT,
    CMP_GE
};

struct CallFunctionOperand {
    std::string funcName;
    int argCount;
};

struct Bytecode {
    BytecodeOp op;
    std::variant<BigNum, std::string, CompareOp, CallFunctionOperand> operand;
};

using BytecodeProgram = std::vector<Bytecode>;

#endif