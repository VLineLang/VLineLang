#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include "../parser/value.hpp"
#include "../parser/errors.hpp"

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
        case Value::OBJECT: {
            printf("{");
            for (auto& ObjMem : value.objectMembers) {
                printf("%s: ", ObjMem.first.c_str());
                printValue(ObjMem.second);
                printf(", ");
            }
            printf("}");
            break;
        }
        case Value::NULL_TYPE: printf("null"); break;
    }
}

#endif