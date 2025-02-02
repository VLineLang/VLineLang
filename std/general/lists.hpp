#ifndef STD_LISTS_HPP
#define STD_LISTS_HPP

#include "../../utils/core.hpp"

Value builtinRange(const std::vector<Value>& args) {
    checkArgCount("range", 2, args);
    if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
        throwTypeError("range() expects number");
    }

    std::vector<Value> list;
    BigNum start = args[0].bignumValue;
    BigNum end = args[1].bignumValue;
    for (BigNum i = start; i < end; i = i + 1) {
        list.emplace_back(i);
    }
    return Value(list);
}

Value builtinLen(const std::vector<Value>& args) {
    checkArgCount("len", 1, args);
    if (args[0].type == Value::STRING) {
        return Value(args[0].strValue.size());
    }
    if (args[0].type == Value::LIST) {
        return Value(args[0].listValue.size());
    }
    throwTypeError("len() expects string or list");
}

#endif