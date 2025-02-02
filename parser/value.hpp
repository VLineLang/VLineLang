#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include "../vm/bignum.hpp"

struct Value {
    enum ValueType { NUMBER, STRING, LIST, NULL_TYPE };
    ValueType type;
    std::string strValue;
    std::vector<Value> listValue;
    BigNum bignumValue;

    Value() : type(NULL_TYPE) {}
    explicit Value(const BigNum& val) : type(NUMBER), bignumValue(val) {}
    explicit Value(const std::string& val) : type(STRING), strValue(val) {}
    explicit Value(const std::vector<Value>& val) : type(LIST), listValue(val) {}
};

#endif