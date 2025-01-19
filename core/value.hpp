#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>
#include <stdexcept>

struct Value {
    enum ValueType { NUMBER, STRING, LIST, NULL_TYPE };
    ValueType type;
    double numValue;
    std::string strValue;
    std::vector<Value> listValue;

    Value() : type(NUMBER), numValue(0.0) {}
    Value(double value) : type(NUMBER), numValue(value) {}
    Value(const std::string& value) : type(STRING), strValue(value) {}
    Value(const std::vector<Value>& value) : type(LIST), listValue(value) {}
    Value(ValueType type) : type(type) {}
};

class ReturnException : public std::exception {
public:
    Value value;

    ReturnException(Value val) : value(val) {}
};

class BreakException : public std::exception {
public:
    BreakException() = default;
};

class ContinueException : public std::exception {
public:
    ContinueException() = default;
};

#endif