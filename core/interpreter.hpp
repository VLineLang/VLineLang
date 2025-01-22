#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "vm.hpp"
#include "codegen.hpp"
#include "ast.hpp"
#include <vector>

#include <iostream>
#include <fstream>

void printValue(const Value& value) {
    if (value.type == Value::STRING) {
        std::cout << value.strValue;
    } else if (value.type == Value::NUMBER) {
        std::cout << value.numValue;
    } else if (value.type == Value::LIST) {
        std::cout << "[";
        for (size_t i = 0; i < value.listValue.size(); ++i) {
            printValue(value.listValue[i]);
            if (i < value.listValue.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    } else if (value.type == Value::NULL_TYPE) {
        std::cout << "null";
    }
}
#endif