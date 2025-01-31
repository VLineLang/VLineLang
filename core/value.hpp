#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>
#include <stdexcept>

struct Value {
    enum ValueType { INT, DOUBLE, STRING, LIST, NULL_TYPE };
    ValueType type;
    std::string strValue;
    std::vector<Value> listValue;

    union {
        int intValue;
        double doubleValue;
    };

    Value() : type(NULL_TYPE) {}
    explicit Value(long long val) : type(INT), intValue(val) {}
    explicit Value(int val) : type(INT), intValue(val) {}
    explicit Value(unsigned long long val) : type(INT), intValue(val) {}
    explicit Value(float val) : type(DOUBLE), doubleValue(val) {}
    explicit Value(double val) : type(DOUBLE), doubleValue(val) {}
    explicit Value(const std::string& val) : type(STRING), strValue(val) {}
    explicit Value(const std::vector<Value>& val) : type(LIST), listValue(val) {}
};

#endif