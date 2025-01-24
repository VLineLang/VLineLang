#ifndef VM_HPP
#define VM_HPP

#include "core.hpp"
#include <deque>
#include <unordered_map>
#include <vector>
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
        case Value::NUMBER: printf((value.numValue-(int)(value.numValue)?"%lf":"%.0lf"), value.numValue); break;
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
        std::unordered_map<std::string, Value> locals;
        Frame* parent;
        BytecodeProgram program;
        size_t pc;
        Value returnValue;


        Frame(const BytecodeProgram& program, Frame* parent = nullptr)
                : program(program), pc(0), parent(parent) {}

        ~Frame() {}
    };

    std::deque<Frame> frames;
    std::vector<Value> operandStack;
    std::unordered_map<std::string, FunctionDeclaration*> functions;

    void setDepthLimit(size_t depth) {
        maxRecursionDepth = depth;
    }

//    void debugVMState(const std::deque<Frame>& frames, const std::vector<Value>& operandStack) {
//        std::cout << "==================== Debug VM State ====================" << std::endl;
//
//
//        std::cout << "Operand Stack (" << operandStack.size() << " elements):" << std::endl;
//        for (size_t i = 0; i < operandStack.size(); ++i) {
//            std::cout << "  [" << i << "]: ";
//            printValue(operandStack[i]);
//            std::cout << std::endl;
//        }
//
//
//        std::cout << "Frame Stack (" << frames.size() << " frames):" << std::endl;
//        for (size_t i = 0; i < frames.size(); ++i) {
//            const VM::Frame& frame = frames[i];
//            std::cout << "  Frame " << i << " (PC: " << frame.pc << "):" << std::endl;
//
//
//            std::cout << "    Locals (" << frame.locals.size() << " variables):" << std::endl;
//            for (const auto& [name, value] : frame.locals) {
//                std::cout << "      " << name << " = ";
//                printValue(value);
//                std::cout << std::endl;
//            }
//
//
//            if (frame.pc < frame.program.size()) {
//                const Bytecode& instr = frame.program[frame.pc];
//                std::cout << "    Current Instruction: ";
//                switch (instr.op) {
//                    case LOAD_CONST:
//                        if (auto pval = std::get_if<double>(&instr.operand)) {
//                            std::cout << "LOAD_CONST " << *pval;
//                        } else if (auto sval = std::get_if<std::string>(&instr.operand)) {
//                            std::cout << "LOAD_CONST \"" << *sval << "\"";
//                        }
//                        break;
//                    case LOAD_VAR:
//                        std::cout << "LOAD_VAR " << std::get<std::string>(instr.operand);
//                        break;
//                    case STORE_VAR:
//                        std::cout << "STORE_VAR " << std::get<std::string>(instr.operand);
//                        break;
//                    case BINARY_OP:
//                        std::cout << "BINARY_OP " << std::get<std::string>(instr.operand);
//                        break;
//                    case JUMP:
//                        std::cout << "JUMP " << std::get<int>(instr.operand);
//                        break;
//                    case JUMP_IF_FALSE:
//                        std::cout << "JUMP_IF_FALSE " << std::get<int>(instr.operand);
//                        break;
//                    case CALL_FUNCTION: {
//                        auto op = std::get<CallFunctionOperand>(instr.operand);
//                        std::cout << "CALL_FUNCTION " << op.funcName << " (" << op.argCount << " args)";
//                        break;
//                    }
//                    case RETURN:
//                        std::cout << "RETURN";
//                        break;
//                    case BUILD_LIST:
//                        std::cout << "BUILD_LIST " << std::get<int>(instr.operand);
//                        break;
//                    case GET_ITER:
//                        std::cout << "GET_ITER";
//                        break;
//                    case FOR_ITER:
//                        std::cout << "FOR_ITER " << std::get<int>(instr.operand);
//                        break;
//                    case POP:
//                        std::cout << "POP";
//                        break;
//                    case LOAD_SUBSCRIPT:
//                        std::cout << "LOAD_SUBSCRIPT";
//                        break;
//                    case STORE_SUBSCRIPT:
//                        std::cout << "STORE_SUBSCRIPT";
//                        break;
//                    default:
//                        std::cout << "UNKNOWN_OP";
//                        break;
//                }
//                std::cout << std::endl;
//            } else {
//                std::cout << "    Current Instruction: END OF PROGRAM" << std::endl;
//            }
//        }
//
//        std::cout << "========================================================" << std::endl;
//    }

    Value execute() {
        if (frames.empty()) {
            return Value(Value::NULL_TYPE);
        }

        Frame& currentFrame = frames.back();

        while (currentFrame.pc < currentFrame.program.size()) {
            const Bytecode& instr = currentFrame.program[currentFrame.pc];

//            debugVMState(frames, operandStack);

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
                    case POP: operandStack.pop_back(); break;
                    case RETURN: handleReturn(currentFrame); break;
                    case LOAD_SUBSCRIPT: handleLoadSubscript(); break;
                    case STORE_SUBSCRIPT: handleStoreSubscript(); break;
                    default: throwRuntimeError("Unknown bytecode instruction");
                }
                currentFrame.pc++;
            } catch (const std::runtime_error& e) {
                if (frames.size() > 1) {
                    frames.pop_back();
                }
                throw;
            }
        }

        if (frames.size() == 1 && !operandStack.empty()) {
            currentFrame.returnValue = operandStack.back();
            operandStack.pop_back();
        }

        return currentFrame.returnValue;
    }

private:
    size_t maxRecursionDepth = 65536;

    void handleLoadConst(const Bytecode& instr) {
        if (auto pval = std::get_if<double>(&instr.operand)) {
            operandStack.push_back(Value(*pval));
        } else if (auto sval = std::get_if<std::string>(&instr.operand)) {
            operandStack.push_back(Value(*sval));
        } else {
            throwRuntimeError("Invalid LOAD_CONST operand");
        }
    }

    void handleLoadSubscript() {
        if (operandStack.size() < 2) throwRuntimeError("Stack underflow");
        Value index = operandStack.back(); operandStack.pop_back();
        Value list = operandStack.back(); operandStack.pop_back();
        if (list.type != Value::LIST) throwTypeError("Expected list");
        if (index.type != Value::NUMBER) throwTypeError("Index must be number");
        int idx = static_cast<int>(index.numValue);
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        operandStack.push_back(list.listValue[idx]);
    }

    void handleStoreSubscript() {
        if (operandStack.size() < 3) throwRuntimeError("Stack underflow");
        Value value = operandStack.back(); operandStack.pop_back();
        Value index = operandStack.back(); operandStack.pop_back();
        Value list = operandStack.back(); operandStack.pop_back();
        if (list.type != Value::LIST) throwTypeError("Expected list");
        if (index.type != Value::NUMBER) throwTypeError("Index must be number");
        int idx = static_cast<int>(index.numValue);
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        list.listValue[idx] = value;
        operandStack.push_back(list);
    }

    void handleLoadVar(const Bytecode& instr, Frame& frame) {
        const std::string& name = std::get<std::string>(instr.operand);
        Frame* currentFrame = &frame;


        while (currentFrame != nullptr) {
            if (currentFrame->locals.count(name)) {
                operandStack.push_back(currentFrame->locals[name]);
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
        Value value = operandStack.back();
        operandStack.pop_back();
        frame.locals[name] = value;
    }

    void handleBinaryOp(const Bytecode& instr) {
        if (operandStack.size() < 2) {
            throwRuntimeError("Stack underflow in binary operation");
        }
        Value right = operandStack.back(); operandStack.pop_back();
        Value left = operandStack.back(); operandStack.pop_back();
        std::string op = std::get<std::string>(instr.operand);
        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (op == "+") {
                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                    operandStack.push_back(Value(left.numValue + right.numValue));
                } else if (left.type == Value::STRING && right.type == Value::STRING) {
                    operandStack.push_back(Value(left.strValue + right.strValue));
                } else {
                    throwTypeError("Type mismatch in binary operation");
                }
            } else if (op == "-") {
                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                    operandStack.push_back(Value(left.numValue - right.numValue));
                } else {
                    throwTypeError("Type mismatch in binary operation");
                }
            } else if (op == "*") {
                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                    operandStack.push_back(Value(left.numValue * right.numValue));
                } else {
                    throwTypeError("Type mismatch in binary operation");
                }
            } else if (op == "/") {
                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                    if (right.numValue == 0) {
                        throwZeroDivisionError("Zero division error");
                    }
                    operandStack.push_back(Value(left.numValue / right.numValue));
                } else {
                    throwTypeError("Type mismatch in binary operation");
                }
            } else {
                throwRuntimeError("Unknown binary operator: " + op);
            }
        } else if (op == "<" || op == "<=" || op == "==" ||
                  op == "!=" || op == ">" || op == ">=") {
            bool result;
            if (op == "<") result = (left.numValue < right.numValue);
            else if (op == "<=") result = (left.numValue <= right.numValue);
            else if (op == "==") result = (left.numValue == right.numValue);
            else if (op == "!=") result = (left.numValue != right.numValue);
            else if (op == ">") result = (left.numValue > right.numValue);
            else if (op == ">=") result = (left.numValue >= right.numValue);

            operandStack.push_back(Value(result ? 1.0 : 0.0));
        } else if (op == "[]") {
            if (left.type != Value::LIST) {
                throwTypeError("Expected list for [] operator");
            }
            if (right.type != Value::NUMBER) {
                throwTypeError("Expected number for list index");
            }
            int index = static_cast<int>(right.numValue);
            if (index < 0 || index >= left.listValue.size()) {
                throwIndexError("List index out of range");
            }
            operandStack.push_back(left.listValue[index]);
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
        Value cond = operandStack.back(); operandStack.pop_back();
        if (cond.numValue == 0.0) {
            return std::get<int>(instr.operand);
        }
        return pc + 1;
    }

    void handleBuildList(const Bytecode& instr) {
        int count = std::get<int>(instr.operand);
        if (operandStack.size() < count) {
            throwRuntimeError("Stack underflow in list construction");
        }
        std::deque<Value> elements;
        for (int i = 0; i < count; i++) {
            elements.push_front(operandStack.back());
            operandStack.pop_back();
        }
        operandStack.push_back(Value(elements));
    }

    void handleGetIter() {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in GET_ITER");
        }
        Value original = operandStack.back();
        operandStack.pop_back();

        if (original.type != Value::LIST) {
            throwTypeError("Can only iterate over lists");
        }


        Value iterator;
        iterator.type = Value::LIST;
        iterator.listValue = original.listValue;
        operandStack.push_back(iterator);
    }

    size_t handleForIter(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in FOR_ITER");
        }
        Value& iterator = operandStack.back();

        if (iterator.listValue.empty()) {
            operandStack.pop_back();
            return std::get<int>(instr.operand);
        }


        Value current = iterator.listValue.front();
        iterator.listValue.erase(iterator.listValue.begin());
        operandStack.push_back(current);
        return pc + 1;
    }

    void handleCallFunction(const Bytecode& instr) {
        if (frames.size() >= maxRecursionDepth) {
            throwRuntimeError("Maximum recursion depth exceeded");
        }

        CallFunctionOperand op = std::get<CallFunctionOperand>(instr.operand);
        std::deque<Value> args;
        for (int i = 0; i < op.argCount; i++) {
            if (operandStack.empty()) {
                throwRuntimeError("Stack underflow in function call");
            }
            args.push_front(operandStack.back());
            operandStack.pop_back();
        }

        if (functions.count(op.funcName)) {
            FunctionDeclaration* func = functions[op.funcName];
            if (func->parameters.size() != args.size()) {
                throwTypeError("Function " + op.funcName + " expects " +
                               std::to_string(func->parameters.size()) + " arguments, but got " +
                               std::to_string(args.size()));
            }


            frames.emplace_back(func->bytecode, &frames.back());
            Frame& childFrame = frames.back();


            for (size_t i = 0; i < func->parameters.size(); ++i) {
                childFrame.locals[func->parameters[i]] = args[i];
            }


            execute();


            Value returnValue = childFrame.returnValue;
            frames.pop_back();


            operandStack.push_back(returnValue);
        } else {
            operandStack.push_back(callBuiltinFunction(op.funcName, args));
        }
    }

    Value callBuiltinFunction(const std::string& name, const std::deque<Value>& args) {
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
        } else if (name.find(".") != std::string::npos) {
            size_t dotPos = name.find(".");
            std::string objectName = name.substr(0, dotPos);
            std::string methodName = name.substr(dotPos + 1);

            Value objectValue;
            try {
                objectValue = frames.back().locals.at(objectName);
            } catch (const std::out_of_range&) {
                throwIdentifierError("Undefined variable: " + objectName);
            }

            std::deque<Value> newArgs = args;
            newArgs.push_front(objectValue);

            Value result;
            if (methodName == "size") {
                result = listSize(newArgs);
            } else if (methodName == "empty") {
                result = listEmpty(newArgs);
            } else if (methodName == "append") {
                result = listAppend(newArgs);
            } else if (methodName == "insert") {
                result = listInsert(newArgs);
            } else if (methodName == "erase") {
                result = listErase(newArgs);
            } else {
                throwIdentifierError("Undefined member function: " + methodName);
            }

            if (methodName == "append" || methodName == "insert" || methodName == "erase") {
                frames.back().locals[objectName] = result;
            }

            return result;
        } else {
            throwIdentifierError("Undefined builtin function: " + name);
        }
    }


    void handleReturn(Frame& frame) {
        if (!operandStack.empty()) {
            frame.returnValue = operandStack.back();
            operandStack.pop_back();
        } else {
            frame.returnValue = Value(Value::NULL_TYPE);
        }
        frame.pc = frame.program.size();
    }

    Value listSize(const std::deque<Value>& args) {
        checkArgCount("list.size", 1, args);
        if (args[0].type != Value::LIST) {
            throwTypeError("list.size() expects a list");
        }
        return Value(static_cast<double>(args[0].listValue.size()));
    }

    Value listEmpty(const std::deque<Value>& args) {
        checkArgCount("list.empty", 1, args);
        if (args[0].type != Value::LIST) {
            throwTypeError("list.empty() expects a list");
        }
        return Value(args[0].listValue.empty() ? 1.0 : 0.0);
    }

    Value listAppend(const std::deque<Value>& args) {
        checkArgCount("list.append", 2, args);
        if (args[0].type != Value::LIST) {
            throwTypeError("list.append() expects a list");
        }
        Value listCopy = args[0];
        listCopy.listValue.push_back(args[1]);
        return listCopy;
    }

    Value listInsert(const std::deque<Value>& args) {
        checkArgCount("list.insert", 3, args);
        if (args[0].type != Value::LIST) {
            throwTypeError("list.insert() expects a list");
        }
        if (args[1].type != Value::NUMBER) {
            throwTypeError("list.insert() expects a number as the index");
        }
        int index = static_cast<int>(args[1].numValue);
        if (index < 0 || index > args[0].listValue.size()) {
            throwIndexError("Index out of range in list.insert()");
        }
        Value listCopy = args[0];
        listCopy.listValue.insert(listCopy.listValue.begin() + index, args[2]);
        return listCopy;
    }

    Value listErase(const std::deque<Value>& args) {
        checkArgCount("list.erase", 3, args);
        if (args[0].type != Value::LIST) {
            throwTypeError("list.erase() expects a list");
        }
        if (args[1].type != Value::NUMBER || args[2].type != Value::NUMBER) {
            throwTypeError("list.erase() expects numbers as the range");
        }
        int begin = static_cast<int>(args[1].numValue);
        int end = static_cast<int>(args[2].numValue);
        if (begin < 0 || end > args[0].listValue.size() || begin > end) {
            throwIndexError("Invalid range in list.erase()");
        }
        Value listCopy = args[0];
        listCopy.listValue.erase(listCopy.listValue.begin() + begin, listCopy.listValue.begin() + end);
        return listCopy;
    }

    Value builtinPrint(const std::deque<Value>& args) {
        std::string end = "\n", step = " ";
        if (args.size() >= 2) {
            if (args[args.size() - 2].type == Value::STRING) {
                end = args[args.size() - 2].strValue;
            } else {
                throwTypeError("print() expects a string as the second-to-last argument");
            }
            if (args[args.size() - 1].type == Value::STRING) {
                step = args[args.size() - 1].strValue;
            } else {
                throwTypeError("print() expects a string as the last argument");
            }
        }
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) printf("%s", step.c_str());
            printValue(args[i]);
        }
        printf("%s", end.c_str());
        return Value(Value::NULL_TYPE);
    }

    Value builtinInput(const std::deque<Value>& args) {
        if (!args.empty()) {
            printValue(args[0]);
        }
        std::string input;
        std::getline(std::cin, input);
        return Value(input);
    }

    Value builtinLen(const std::deque<Value>& args) {
        checkArgCount("len", 1, args);
        if (args[0].type == Value::STRING) {
            return Value(static_cast<double>(args[0].strValue.size()));
        }
        if (args[0].type == Value::LIST) {
            return Value(static_cast<double>(args[0].listValue.size()));
        }
        throwTypeError("len() expects string or list");
    }

    Value builtinType(const std::deque<Value>& args) {
        checkArgCount("type", 1, args);
        switch (args[0].type) {
            case Value::NUMBER: return Value("number");
            case Value::STRING: return Value("string");
            case Value::LIST: return Value("list");
            case Value::NULL_TYPE: return Value("null");
            default: return Value("unknown");
        }
    }

    Value builtinRange(const std::deque<Value>& args) {
        checkArgCount("range", 2, args);
        if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
            throwTypeError("range() expects numbers");
        }

        std::deque<Value> list;
        int start = static_cast<int>(args[0].numValue);
        int end = static_cast<int>(args[1].numValue);
        for (int i = start; i < end; ++i) {
            list.emplace_back(static_cast<double>(i));
        }
        return Value(list);
    }

    Value builtinSleep(const std::deque<Value>& args) {
        checkArgCount("sleep", 1, args);
        if (args[0].type != Value::NUMBER) {
            throwTypeError("sleep() expects a number");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(args[0].numValue)));
        return Value(Value::NULL_TYPE);
    }

    Value builtinSystem(const std::deque<Value>& args) {
        checkArgCount("system", 1, args);
        if (args[0].type != Value::STRING) {
            throwTypeError("system() expects a string");
        }
        int result = std::system(args[0].strValue.c_str());
        return Value(static_cast<double>(result));
    }

    Value builtinExit(const std::deque<Value>& args) {
        checkArgCount("exit", 1, args);
        if (args[0].type != Value::NUMBER) {
            throwTypeError("exit() expects a number");
        }
        std::exit(static_cast<int>(args[0].numValue));
    }

    Value builtinRead(const std::deque<Value>& args) {
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

    Value builtinWrite(const std::deque<Value>& args) {
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
        return Value(static_cast<double>(std::time(nullptr)));
    }

    void checkArgCount(const std::string& func, int expected, const std::deque<Value>& args, int minn = 0) {
        if (expected >= 0 && args.size() != expected) {
            throwTypeError(func + "() expects " + std::to_string(expected) + " arguments");
        } else {
            if (args.size() < minn) {
                throwTypeError(func + "() expects at least " + std::to_string(minn) + " arguments");
            }
        }
    }
};

#endif