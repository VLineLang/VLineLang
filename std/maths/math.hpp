#ifndef STD_MATH_HPP
#define STD_MATH_HPP

#include <cmath>
#include "../../utils/utils.hpp"

Value builtinMathFloor(const std::vector<Value>& args) {
    checkArgCount("floor", 1, args);
    if (args[0].type != Value::NUMBER) {
        throwTypeError("floor() expects a number");
    }
    return Value(args[0].bignumValue.trunc());
}

Value builtinMathCeil(const std::vector<Value>& args) {
    checkArgCount("ceil", 1, args);
    if (args[0].type!= Value::NUMBER) {
        throwTypeError("ceil() expects a number");
    }
    return Value(args[0].bignumValue.trunc() == args[0].bignumValue ? args[0].bignumValue : args[0].bignumValue.trunc() + 1);
}

Value builtinMathRound(const std::vector<Value>& args) {
    checkArgCount("round", 1, args);
    if (args[0].type!= Value::NUMBER) {
        throwTypeError("round() expects a number");
    }
    return Value(args[0].bignumValue.trunc() + (args[0].bignumValue - args[0].bignumValue.trunc() >= 0.5? 1 : 0));
}

Value builtinMathAbs(const std::vector<Value>& args) {
    checkArgCount("abs", 1, args);
    if (args[0].type!= Value::NUMBER) {
        throwTypeError("abs() expects a number");
    }
    return Value(args[0].bignumValue.abs());
}

Value builtinMathSqrt(const std::vector<Value>& args) {
    checkArgCount("sqrt", 1, args);
    if (args[0].type!= Value::NUMBER) {
        throwTypeError("sqrt() expects a number");
    }
    return Value(args[0].bignumValue.sqrt());
}

Value builtinMathPow(const std::vector<Value>& args) {
    checkArgCount("pow", 2, args);
    if (args[0].type!= Value::NUMBER || args[1].type!= Value::NUMBER) {
        throwTypeError("pow() expects two numbers");
    }
    return Value(args[0].bignumValue.pow(args[1].bignumValue));
}

#endif