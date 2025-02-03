#ifndef STD_SYS_HPP
#define STD_SYS_HPP

#include "../../utils/utils.hpp"

Value builtinSystem(const std::vector<Value>& args) {
    checkArgCount("system", 1, args);
    if (args[0].type != Value::STRING) {
        throwTypeError("system() expects a string");
    }
    int result = std::system(args[0].strValue.c_str());
    return Value(result);
}

Value builtinExit(const std::vector<Value>& args) {
    checkArgCount("exit", 1, args);
    if (args[0].type != Value::NUMBER) {
        throwTypeError("exit() expects a number");
    }
    std::exit(args[0].bignumValue.get_ll());
}

#endif