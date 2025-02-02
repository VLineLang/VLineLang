#ifndef CORE_HPP
#define CORE_HPP

#define VLINE_VERSION "0.5.4-alpha.1"
#define VLINE_PUBLISH "Feb. 1st, 2025"
#define VLINE_COMPILER "GNU GCC/ISO C++17"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../parser/errors.hpp"
#include "../parser/value.hpp"

void checkArgCount(const std::string& func, size_t expected, const std::vector<Value>& args) {
    if (args.size() != expected) {
        throwTypeError(func + "() expects " + std::to_string(expected) + " arguments");
    }
}

void printValue(const Value& value) {
    switch (value.type) {
        case Value::NUMBER: printf("%s", value.bignumValue.to_string().c_str()); break;
        case Value::STRING: printf("%s", value.strValue.c_str()); break;
        case Value::LIST: {
            printf("[");
            for (size_t i = 0; i < value.listValue.size(); ++i) {
                printValue(value.listValue[i]);
                if (i < value.listValue.size() - 1) printf(", ");
            }
            printf("]");
            break;
        }
        case Value::NULL_TYPE: printf("null"); break;
    }
}

#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../parser/parser.hpp"
#include "../vm/vm.hpp"
#include "../bytecode/bytecode.hpp"
#include "../bytecode/codegen.hpp"
#include "../vm/bignum.hpp"

#endif