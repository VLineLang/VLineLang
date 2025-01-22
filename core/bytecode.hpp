// bytecode.hpp
#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <vector>
#include <string>
#include <variant>

enum BytecodeOp {
    LOAD_CONST,
    LOAD_VAR,
    STORE_VAR,
    BINARY_OP,
    UNARY_OP,
    JUMP,
    JUMP_IF_FALSE,
    JUMP_IF_TRUE,
    CALL_FUNCTION,
    RETURN,
    POP,
    COMPARE_OP,
    BUILD_LIST,
    GET_ITER,
    FOR_ITER,
    JUMP_ABSOLUTE,
    POP_JUMP_IF_FALSE,
    LOAD_GLOBAL,
    STORE_GLOBAL
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
    std::variant<double, std::string, int, CompareOp, CallFunctionOperand> operand;
};

using BytecodeProgram = std::vector<Bytecode>;

#endif