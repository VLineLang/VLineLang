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
        case Value::OBJECT: return Value("object");
        default: return Value("unknown");
    }
}

Value builtinNumber(const std::vector<Value>& args) {
    checkArgCount("number", 1, args);

    switch (args[0].type) {
        case Value::NUMBER: {
            return Value(args[0].bignumValue);
        }
        case Value::STRING: {
            return Value(BigNum(args[0].strValue));
        }
        case Value::NULL_TYPE: {
            return Value(BigNum());
        }
        default: {
            throwTypeError("Cannot convert to int!");
            return Value();
        }
    }
}

Value builtinStr(const std::vector<Value>& args) {
    checkArgCount("str", 1, args);

    switch (args[0].type) {
        case Value::NUMBER: {
            return Value(args[0].bignumValue.to_string());
        }
        case Value::STRING: {
            return Value(args[0].strValue);
        }
        case Value::NULL_TYPE: {
            return Value("null");
        }
        default: {
            throwTypeError("Cannot convert to string!");
            return Value();
        }
    }
}

Value builtinList(const std::vector<Value>& args) {
    checkArgCount("list", 1, args);
    switch (args[0].type) {
        case Value::LIST: {
            return Value(args[0].listValue);
        }
        case Value::STRING: {
            std::vector<Value> list;
            for (char c : args[0].strValue) {
                list.push_back(Value(std::string(1, c)));
            }
            return Value(list);
        }
        case Value::NULL_TYPE: {
            return Value(std::vector<Value>());
        }
        case Value::NUMBER: {
            std::vector<Value> list;
            list.push_back(Value(args[0].bignumValue));
            return Value(list);
        }
        default: {
            throwTypeError("Cannot convert to list!");
            return Value();
        }
    }
}

#endif