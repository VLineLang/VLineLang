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

Value listAppend(const std::vector<Value>& args) {
    checkArgCount("list.append", 2, args);
    if (args[0].type != Value::LIST) {
        throwTypeError("list.append() expects a list");
    }

    Value listCopy = args[0];
    listCopy.listValue.push_back(args[1]);
    return listCopy;
}

Value listInsert(const std::vector<Value>& args) {
    checkArgCount("list.insert", 3, args);
    if (args[0].type!= Value::LIST) {
        throwTypeError("list.insert() expects a list");
    }
    if (args[1].type!= Value::NUMBER || args[2].type!= Value::NUMBER) {
        throwTypeError("list.insert() expects two numbers");
    }
    Value listCopy = args[0];
    BigNum index = args[1].bignumValue;
    const Value& value = args[2];
    if (index < 0 || index > listCopy.listValue.size()) {
        throwIndexError("list.insert() index out of range");
    }
    listCopy.listValue.insert(listCopy.listValue.begin() + index.get_ll(), value);
    return listCopy;
}

Value listErase(const std::vector<Value>& args) {
    checkArgCount("list.erase", 3, args);
    if (args[0].type!= Value::LIST) {
        throwTypeError("list.erase() expects a list");
    }
    if (args[1].type != Value::NUMBER || args[2].type != Value::NUMBER) {
        throwTypeError("list.erase() expects two numbers");
    }
    Value listCopy = args[0];
    BigNum start = args[1].bignumValue;
    BigNum end = args[2].bignumValue;
    if (start < 0 || start >= listCopy.listValue.size() || end < 0 || end > listCopy.listValue.size()) {
        throwIndexError("list.erase() index out of range");
    }
    listCopy.listValue.erase(listCopy.listValue.begin() + start.get_ll(), listCopy.listValue.begin() + end.get_ll());
    return listCopy;
}

#endif