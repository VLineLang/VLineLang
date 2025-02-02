#ifndef STD_TIME_HPP
#define STD_TIME_HPP

#include "../../utils/core.hpp"
#include <chrono>
#include <thread>

Value builtinSleep(const std::vector<Value>& args) {
    checkArgCount("sleep", 1, args);
    if (args[0].type != Value::NUMBER) {
        throwTypeError("sleep() expects a number");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(args[0].bignumValue.get_ll()));
    return Value();
}

Value builtinTime() {
    return Value(std::time(nullptr));
}

#endif