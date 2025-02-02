#ifndef STD_IO_HPP
#define STD_IO_HPP

#include "../../utils/core.hpp"

Value builtinPrint(const std::vector<Value>& args) {
    for (const auto & arg : args) {
        printValue(arg);
    }
    return Value();
}

Value builtinInput(const std::vector<Value>& args) {
    if (!args.empty()) {
        printValue(args[0]);
    }
    std::string input;
    std::getline(std::cin, input);
    return Value(input);
}

#endif