#ifndef STD_TYPE_HPP
#define STD_TYPE_HPP

#include "../../utils/core.hpp"

Value builtinType(const std::vector<Value>& args) {
    checkArgCount("type", 1, args);
    switch (args[0].type) {
        case Value::NUMBER: return Value("number");
        case Value::STRING: return Value("string");
        case Value::LIST: return Value("list");
        case Value::NULL_TYPE: return Value("null");
        default: return Value("unknown");
    }
}

#endif