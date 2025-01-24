#ifndef VALUE_HPP
#define VALUE_HPP

#include <deque>
#include <string>
#include <stdexcept>

struct Value {
    enum ValueType { NUMBER, STRING, LIST, NULL_TYPE };
    ValueType type;
    double numValue;
    std::string strValue;
    std::deque<Value> listValue;

    Value() : type(NUMBER), numValue(0.0) {}
    Value(double value) : type(NUMBER), numValue(value) {}
    Value(const std::string& value) : type(STRING), strValue(value) {}
    Value(const std::deque<Value>& value) : type(LIST), listValue(value) {}
    Value(ValueType type) : type(type) {}
    Value(const Value& other) : type(other.type) {
        switch (type) {
            case LIST:
                listValue = other.listValue;
                break;
            case STRING:
                new (&strValue) std::string(other.strValue);
                break;
            default:
                numValue = other.numValue;
        }
    }

    Value& operator=(const Value& other) {
        if (this == &other) return *this;

        if (type == STRING) strValue.~basic_string();

        type = other.type;
        switch (type) {
            case LIST:
                listValue = other.listValue;
                break;
            case STRING:
                new (&strValue) std::string(other.strValue);
                break;
            default:
                numValue = other.numValue;
        }
        return *this;
    }
};
#endif