#ifndef STD_FILES_HPP
#define STD_FILES_HPP

#include "../../utils/core.hpp"

Value builtinRead(const std::vector<Value>& args) {
    checkArgCount("read", 1, args);
    if (args[0].type != Value::STRING) {
        throwTypeError("read() expects a string");
    }
    std::ifstream file(args[0].strValue);
    if (!file.is_open()) {
        throwIOError("Could not open file: " + args[0].strValue);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return Value(content);
}

Value builtinWrite(const std::vector<Value>& args) {
    checkArgCount("write", 2, args);
    if (args[0].type != Value::STRING || args[1].type != Value::STRING) {
        throwTypeError("write() expects two strings");
    }
    std::ofstream file(args[0].strValue);
    if (!file.is_open()) {
        throwIOError("Could not open file: " + args[0].strValue);
    }
    file << args[1].strValue;
    return Value();
}

#endif