#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include "../vm/bignum.hpp"
#include "../ast/ast.hpp"

struct Value {
    enum ValueType { NUMBER, STRING, LIST, NULL_TYPE, OBJECT };
    ValueType type;
    std::string strValue;
    std::vector<Value> listValue;
    BigNum bignumValue;
    std::map<std::string, Value> objectMembers;
    std::map<std::string, FunctionDeclaration*> functions;


    Value() : type(NULL_TYPE) {}
    explicit Value(const BigNum& val) : type(NUMBER), bignumValue(val) {}
    explicit Value(const std::string& val) : type(STRING), strValue(val) {}
    explicit Value(const std::vector<Value>& val) : type(LIST), listValue(val) {}
};

#endif