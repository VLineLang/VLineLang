#ifndef VM_HPP
#define VM_HPP

#include "bytecode.hpp"
#include "value.hpp"
#include "errors.hpp"
#include "codegen.hpp"
#include <vector>
#include <map>
#include <stack>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <memory>

void printValue(const Value& value) {
    switch (value.type) {
        case Value::INT: printf("%d", value.intValue); break;
        case Value::DOUBLE: printf("%lf", value.doubleValue); break;
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
        case Value::NULL_TYPE: printf("null"); break;
    }
}

class VM {
public:
    struct Frame {
        std::map<std::string, Value> locals;
        Frame* parent;
        BytecodeProgram program;
        size_t pc;
        Value returnValue;


        Frame(const BytecodeProgram& program, Frame* parent = nullptr)
                : program(program), pc(0), parent(parent) {}
    };

    std::stack<Frame> frames;
    std::stack<Value> operandStack;
    std::map<std::string, FunctionDeclaration*> functions;

    void setDepthLimit(size_t depth) {
        maxRecursionDepth = depth;
    }

    Value execute() {
        if (frames.empty()) {
            return Value(Value::NULL_TYPE);
        }

        Frame& currentFrame = frames.top();

        while (currentFrame.pc < currentFrame.program.size()) {
            const Bytecode& instr = currentFrame.program[currentFrame.pc];

            try {
                switch (instr.op) {
                    case LOAD_CONST: handleLoadConst(instr); break;
                    case LOAD_VAR: handleLoadVar(instr, currentFrame); break;
                    case STORE_VAR: handleStoreVar(instr, currentFrame); break;
                    case BINARY_OP: handleBinaryOp(instr); break;
                    case JUMP: currentFrame.pc = handleJump(instr); continue;
                    case JUMP_IF_FALSE: currentFrame.pc = handleJumpIfFalse(instr, currentFrame.pc); continue;
                    case CALL_FUNCTION: handleCallFunction(instr); break;
                    case BUILD_LIST: handleBuildList(instr); break;
                    case GET_ITER: handleGetIter(); break;
                    case FOR_ITER: currentFrame.pc = handleForIter(instr, currentFrame.pc); continue;
                    case POP: operandStack.pop(); break;
                    case RETURN: handleReturn(currentFrame); break;
                    case LOAD_SUBSCRIPT: handleLoadSubscript(); break;
                    case STORE_SUBSCRIPT: handleStoreSubscript(); break;
                    default: throwRuntimeError("Unknown bytecode instruction");
                }
                currentFrame.pc++;
            } catch (const std::runtime_error& e) {
                if (frames.size() > 1) {
                    frames.pop();
                }
                throw;
            }
        }

        return currentFrame.returnValue;
    }

private:
    size_t maxRecursionDepth = 65536;

    void handleLoadConst(const Bytecode& instr) {
        if (auto ival = std::get_if<int>(&instr.operand)) {
            operandStack.push(Value(*ival));
        } else if (auto dval = std::get_if<double>(&instr.operand)) {
            operandStack.push(Value(*dval));
        } else if (auto sval = std::get_if<std::string>(&instr.operand)) {
            operandStack.push(Value(*sval));
        } else {
            throwRuntimeError("Invalid LOAD_CONST operand");
        }
    }

    void handleLoadSubscript() {
        if (operandStack.size() < 2) throwRuntimeError("Stack underflow");
        Value index = operandStack.top(); operandStack.pop();
        Value list = operandStack.top(); operandStack.pop();
        if (list.type != Value::LIST) throwTypeError("Expected list");
        if (index.type != Value::INT) throwTypeError("Index must be integer");
        int idx = index.intValue;
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        operandStack.push(list.listValue[idx]);
    }

    void handleStoreSubscript() {
        if (operandStack.size() < 3) throwRuntimeError("Stack underflow");
        Value value = operandStack.top(); operandStack.pop();
        Value index = operandStack.top(); operandStack.pop();
        Value list = operandStack.top(); operandStack.pop();
        if (list.type != Value::LIST) throwTypeError("Expected list");
        if (index.type != Value::INT) throwTypeError("Index must be integer");
        int idx = index.intValue;
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        list.listValue[idx] = value;
        operandStack.push(list);
    }

    void handleLoadVar(const Bytecode& instr, Frame& frame) {
        const std::string& name = std::get<std::string>(instr.operand);
        Frame* currentFrame = &frame;


        while (currentFrame != nullptr) {
            if (currentFrame->locals.count(name)) {
                operandStack.push(currentFrame->locals[name]);
                return;
            }
            currentFrame = currentFrame->parent;
        }


        throwIdentifierError("Undefined variable '" + name + "'");
    }

    void handleStoreVar(const Bytecode& instr, Frame& frame) {
        const std::string& name = std::get<std::string>(instr.operand);
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in store operation");
        }
        Value value = operandStack.top();
        operandStack.pop();
        frame.locals[name] = value;
    }

    void handleBinaryOp(const Bytecode& instr) {
        if (operandStack.size() < 2) {
            throwRuntimeError("Stack underflow in binary operation");
        }
        Value right = operandStack.top(); operandStack.pop();
        Value left = operandStack.top(); operandStack.pop();
        std::string op = std::get<std::string>(instr.operand);

        auto promoteTypes = [](Value& a, Value& b) {
            if (a.type == Value::INT && b.type == Value::INT) return;
            if (a.type == Value::INT) {
                a.type = Value::DOUBLE;
                a.doubleValue = a.intValue;
            }
            if (b.type == Value::INT) {
                b.type = Value::DOUBLE;
                b.doubleValue = b.intValue;
            }
        };

        auto performArithmetic = [&](auto op) {
            if (left.type == Value::INT && right.type == Value::INT) {
                operandStack.push(Value(op(left.intValue, right.intValue)));
            } else {
                promoteTypes(left, right);
                operandStack.push(Value(op(left.doubleValue, right.doubleValue)));
            }
        };

        auto performComparison = [&](auto cmp) {
            promoteTypes(left, right);
            bool result = cmp(left.doubleValue, right.doubleValue);
            operandStack.push(Value(result ? 1 : 0));
        };

        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (op == "+") {
                if (left.type == Value::STRING && right.type == Value::STRING) {
                    operandStack.push(Value(left.strValue + right.strValue));
                } else {
                    performArithmetic(std::plus<>());
                }
            } else if (op == "-") {
                performArithmetic(std::minus<>());
            } else if (op == "*") {
                if (left.type == Value::STRING && right.type == Value::INT || right.type == Value::STRING && left.type == Value::INT) {
                    bool flag = left.type == Value::STRING && right.type == Value::INT;
                    std::string result;
                    for (int i = 0; i < right.intValue; i++) {
                        if (flag) result += left.strValue;
                        else result += right.strValue;
                    }
                    operandStack.push(Value(result));
                }
                performArithmetic(std::multiplies<>());
            } else if (op == "/") {
                performArithmetic(std::divides<>());
            } else {
                throwRuntimeError("Unknown binary operator: " + op);
            }
        } else if (op == "<" || op == "<=" || op == "==" ||
                  op == "!=" || op == ">" || op == ">=") {
            if (op == "<") {
                performComparison(std::less<>());
            } else if (op == "<=") {
                performComparison(std::less_equal<>());
            } else if (op == "==") {
                performComparison(std::equal_to<>());
            } else if (op == "!=") {
                performComparison(std::not_equal_to<>());
            } else if (op == ">") {
                performComparison(std::greater<>());
            } else if (op == ">=") {
                performComparison(std::greater_equal<>());
            } else {
                throwRuntimeError("Unknown comparison operator: " + op);
            }
        } else if (op == "[]") {
            if (left.type != Value::LIST) {
                throwTypeError("Expected list for [] operator");
            }
            if (right.type != Value::INT) {
                throwTypeError("Expected int for list index");
            }
            int index = right.intValue;
            if (index < 0 || index >= left.listValue.size()) {
                throwIndexError("List index out of range");
            }
            operandStack.push(left.listValue[index]);
        } else {
            throwRuntimeError("Unknown operator: " + op);
        }
    }

    size_t handleJump(const Bytecode& instr) {
        return std::get<int>(instr.operand);
    }

    size_t handleJumpIfFalse(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in jump if false");
        }
        Value cond = operandStack.top(); operandStack.pop();
        if (cond.intValue == 0) {
            return std::get<int>(instr.operand);
        }
        return pc + 1;
    }

    void handleBuildList(const Bytecode& instr) {
        int count = std::get<int>(instr.operand);
        if (operandStack.size() < count) {
            throwRuntimeError("Stack underflow in list construction");
        }
        std::vector<Value> elements;
        for (int i = 0; i < count; i++) {
            elements.insert(elements.begin(), operandStack.top());
            operandStack.pop();
        }
        operandStack.push(Value(elements));
    }

    void handleGetIter() {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in GET_ITER");
        }
        Value original = operandStack.top();
        operandStack.pop();

        if (original.type != Value::LIST) {
            throwTypeError("Can only iterate over lists");
        }


        Value iterator;
        iterator.type = Value::LIST;
        iterator.listValue = original.listValue;
        operandStack.push(iterator);
    }

    size_t handleForIter(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in FOR_ITER");
        }
        Value& iterator = operandStack.top();

        if (iterator.listValue.empty()) {
            operandStack.pop();
            return std::get<int>(instr.operand);
        }


        Value current = iterator.listValue.front();
        iterator.listValue.erase(iterator.listValue.begin());
        operandStack.push(current);
        return pc + 1;
    }

    void handleCallFunction(const Bytecode& instr) {
        CallFunctionOperand op = std::get<CallFunctionOperand>(instr.operand);
        std::vector<Value> args;
        for (int i = 0; i < op.argCount; i++) {
            if (operandStack.empty()) {
                throwRuntimeError("Stack underflow in function call");
            }
            args.insert(args.begin(), operandStack.top());
            operandStack.pop();
        }

        Value result;
        if (functions.count(op.funcName)) {
            FunctionDeclaration* func = functions[op.funcName];
            if (func->parameters.size() != args.size()) {
                throwTypeError("Function " + op.funcName + " expects " +
                               std::to_string(func->parameters.size()) + " arguments, but got " +
                               std::to_string(args.size()));
            }


            Frame newFrame(func->bytecode, &frames.top());

            for (size_t i = 0; i < func->parameters.size(); ++i) {
                newFrame.locals[func->parameters[i]] = args[i];
            }
            frames.push(newFrame);


            result = execute();
            frames.pop();
        } else {
            result = callBuiltinFunction(op.funcName, args);
        }

        operandStack.push(result);
    }

    Value callBuiltinFunction(const std::string& name, const std::vector<Value>& args) {
        if (name == "print") {
            return builtinPrint(args);
        } else if (name == "input") {
            return builtinInput(args);
        } else if (name == "len") {
            return builtinLen(args);
        } else if (name == "type") {
            return builtinType(args);
        } else if (name == "range") {
            return builtinRange(args);
        } else if (name == "sleep") {
            return builtinSleep(args);
        } else if (name == "system") {
            return builtinSystem(args);
        } else if (name == "exit") {
            return builtinExit(args);
        } else if (name == "read") {
            return builtinRead(args);
        } else if (name == "write") {
            return builtinWrite(args);
        } else if (name == "time") {
            return builtinTime();
        } else if (name == "list.append") {
            return listAppend(args);
        } else {
            throwIdentifierError("Undefined builtin function: " + name);
        }
    }


    void handleReturn(Frame& frame) {
        if (!operandStack.empty()) {
            frame.returnValue = operandStack.top();
            operandStack.pop();
        } else {
            frame.returnValue = Value(Value::NULL_TYPE);
        }
        frame.pc = frame.program.size();
    }

    Value builtinPrint(const std::vector<Value>& args) {
        for (const auto & arg : args) {
            printValue(arg);
        }
        return Value(Value::NULL_TYPE);
    }

    Value builtinInput(const std::vector<Value>& args) {
        if (!args.empty()) {
            printValue(args[0]);
        }
        std::string input;
        std::getline(std::cin, input);
        return Value(input);
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

    Value builtinType(const std::vector<Value>& args) {
        checkArgCount("type", 1, args);
        switch (args[0].type) {
            case Value::INT: return Value("int");
            case Value::DOUBLE: return Value("double");
            case Value::STRING: return Value("string");
            case Value::LIST: return Value("list");
            case Value::NULL_TYPE: return Value("null");
            default: return Value("unknown");
        }
    }

    Value builtinRange(const std::vector<Value>& args) {
        checkArgCount("range", 2, args);
        if (args[0].type != Value::INT || args[1].type != Value::INT) {
            throwTypeError("range() expects integers");
        }

        std::vector<Value> list;
        int start = args[0].intValue;
        int end = args[1].intValue;
        for (int i = start; i < end; ++i) {
            list.emplace_back(i);
        }
        return Value(list);
    }

    Value builtinSleep(const std::vector<Value>& args) {
        checkArgCount("sleep", 1, args);
        if (args[0].type != Value::INT) {
            throwTypeError("sleep() expects a integer");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(args[0].intValue));
        return Value(Value::NULL_TYPE);
    }

    Value builtinSystem(const std::vector<Value>& args) {
        checkArgCount("system", 1, args);
        if (args[0].type != Value::STRING) {
            throwTypeError("system() expects a string");
        }
        int result = std::system(args[0].strValue.c_str());
        return Value(result);
    }

    Value builtinExit(const std::vector<Value>& args) {
        checkArgCount("exit", 1, args);
        if (args[0].type != Value::INT) {
            throwTypeError("exit() expects a integer");
        }
        std::exit(args[0].intValue);
        return Value(Value::NULL_TYPE);
    }

    Value builtinRead(const std::vector<Value>& args) {
        checkArgCount("read", 1, args);
        if (args[0].type != Value::STRING) {
            throwTypeError("read() expects a string");
        }
        std::ifstream file(args[0].strValue);
        if (!file.is_open()) {
            throwIOError("Could not open file: " + args[0].strValue);
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return Value(content);
    }

    Value builtinWrite(const std::vector<Value>& args) {
        checkArgCount("write", 2, args);
        if (args[0].type != Value::STRING || args[1].type != Value::STRING) {
            throwTypeError("write() expects two strings");
        }
        std::ofstream file(args[0].strValue);
        if (!file.is_open()) {
            throwIOError("Could not open file: " + args[0].strValue);
        }
        file << args[1].strValue;
        return Value(Value::NULL_TYPE);
    }

    Value builtinTime() {
        return Value(std::time(nullptr));
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

    void checkArgCount(const std::string& func, size_t expected, const std::vector<Value>& args) {
        if (args.size() != expected) {
            throwTypeError(func + "() expects " + std::to_string(expected) + " arguments");
        }
    }
};

#endif