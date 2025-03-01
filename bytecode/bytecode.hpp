#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <vector>
#include <string>
#include <variant>
#include "../vm/bignum.hpp"

enum BytecodeOp {
    LOAD_CONST,
    LOAD_VAR,
    STORE_VAR,
    BINARY_OP,
    JUMP_IF_FALSE,
    CALL_FUNCTION,
    JUMP,
    RETURN,
    BUILD_LIST,
    POP,
    LOAD_SUBSCRIPT,
    STORE_SUBSCRIPT,
    CREATE_OBJECT,
    LOAD_MEMBER,
    STORE_MEMBER,
    LOAD_FUNC,
    STORE_MEMBER_FUNC,
    CLEAR,
    LABEL
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