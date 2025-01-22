// vm.hpp
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

class VM {
public:

    std::map<std::string, Value> globals;
    std::map<std::string, FunctionDeclaration*> functions;
    std::stack<std::map<std::string, Value>> localScopes;
    std::stack<Value> operandStack;


    size_t maxRecursionDepth = 1000;
    size_t maxLoopIterations = 1'000'000;

    Value execute(const BytecodeProgram& program) {
        size_t pc = 0;
        size_t loopCounter = 0;
        localScopes.push({});

        while (pc < program.size()) {
            const Bytecode& instr = program[pc];

            try {
                switch (instr.op) {
                    case LOAD_CONST: handleLoadConst(instr); break;
                    case LOAD_VAR: handleLoadVar(instr); break;
                    case STORE_VAR: handleStoreVar(instr); break;
                    case BINARY_OP: handleBinaryOp(instr); break;
                    case UNARY_OP: handleUnaryOp(instr); break;
                    case COMPARE_OP: handleCompareOp(instr); break;
                    case JUMP: pc = handleJump(instr); continue;
                    case JUMP_IF_FALSE: pc = handleJumpIfFalse(instr, pc); continue;
                    case JUMP_ABSOLUTE: pc = handleJumpAbsolute(instr); continue;
                    case CALL_FUNCTION: handleCallFunction(instr); break;
                    case BUILD_LIST: handleBuildList(instr); break;
                    case GET_ITER: handleGetIter(); break;
                    case FOR_ITER: pc = handleForIter(instr, pc); continue;
                    case POP: operandStack.pop(); break;
                    case RETURN: handleReturn(); break;
                    default: throwRuntimeError("Unknown bytecode instruction");
                }
                pc++;
            } catch (const std::runtime_error& e) {
                localScopes.pop();
                throw;
            }


            if (++loopCounter > maxLoopIterations) {
                throwRuntimeError("Maximum loop iterations exceeded");
            }
        }

        localScopes.pop();
        return operandStack.empty() ? Value(Value::NULL_TYPE) : operandStack.top();
    }

private:

    void handleLoadConst(const Bytecode& instr) {
        if (auto pval = std::get_if<double>(&instr.operand)) {
            operandStack.push(Value(*pval));
        } else if (auto sval = std::get_if<std::string>(&instr.operand)) {
            operandStack.push(Value(*sval));
        } else {
            throwRuntimeError("Invalid LOAD_CONST operand");
        }
    }

    void handleLoadVar(const Bytecode& instr) {
        const std::string& name = std::get<std::string>(instr.operand);
        auto& locals = localScopes.top();

        if (locals.count(name)) {
            operandStack.push(locals[name]);
        } else if (globals.count(name)) {
            operandStack.push(globals[name]);
        } else {
            throwIdentifierError("Undefined variable '" + name + "'");
        }
    }

    void handleStoreVar(const Bytecode& instr) {
        const std::string& name = std::get<std::string>(instr.operand);
        localScopes.top()[name] = operandStack.top();
        operandStack.pop();
    }

    void handleBinaryOp(const Bytecode& instr) {
        if (operandStack.size() < 2) {
            throwRuntimeError("Stack underflow in binary operation");
        }
        Value right = operandStack.top(); operandStack.pop();
        Value left = operandStack.top(); operandStack.pop();
        std::string op = std::get<std::string>(instr.operand);

        if (op == "+") {
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                operandStack.push(Value(left.numValue + right.numValue));
            } else if (left.type == Value::STRING && right.type == Value::STRING) {
                operandStack.push(Value(left.strValue + right.strValue));
            } else {
                throwTypeError("Type mismatch in binary operation");
            }
        } else if (op == "-") {
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                operandStack.push(Value(left.numValue - right.numValue));
            } else {
                throwTypeError("Type mismatch in binary operation");
            }
        } else if (op == "*") {
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                operandStack.push(Value(left.numValue * right.numValue));
            } else {
                throwTypeError("Type mismatch in binary operation");
            }
        } else if (op == "/") {
            if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                if (right.numValue == 0) {
                    throwZeroDivisionError("Zero division error");
                }
                operandStack.push(Value(left.numValue / right.numValue));
            } else {
                throwTypeError("Type mismatch in binary operation");
            }
        } else {
            throwRuntimeError("Unknown binary operator: " + op);
        }
    }

    void handleUnaryOp(const Bytecode& instr) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in unary operation");
        }
        Value value = operandStack.top(); operandStack.pop();
        std::string op = std::get<std::string>(instr.operand);

        if (op == "-") {
            if (value.type == Value::NUMBER) {
                operandStack.push(Value(-value.numValue));
            } else {
                throwTypeError("Unary operator '-' expects a number");
            }
        } else if (op == "not") {
            if (value.type == Value::NUMBER) {
                operandStack.push(Value(value.numValue == 0.0 ? 1.0 : 0.0));
            } else {
                throwTypeError("Unary operator 'not' expects a boolean (number)");
            }
        } else {
            throwRuntimeError("Unknown unary operator: " + op);
        }
    }

    void handleCompareOp(const Bytecode& instr) {
        if (operandStack.size() < 2) {
            throwRuntimeError("Stack underflow in compare operation");
        }
        Value right = operandStack.top(); operandStack.pop();
        Value left = operandStack.top(); operandStack.pop();
        CompareOp cmp = std::get<CompareOp>(instr.operand);

        bool result = false;
        switch (cmp) {
            case CMP_LT: result = (left.numValue < right.numValue); break;
            case CMP_LE: result = (left.numValue <= right.numValue); break;
            case CMP_EQ: result = (left.numValue == right.numValue); break;
            case CMP_NE: result = (left.numValue != right.numValue); break;
            case CMP_GT: result = (left.numValue > right.numValue); break;
            case CMP_GE: result = (left.numValue >= right.numValue); break;
            default: throwRuntimeError("Unknown compare operator");
        }
        operandStack.push(Value(result ? 1.0 : 0.0));
    }

    size_t handleJump(const Bytecode& instr) {
        return std::get<int>(instr.operand);
    }

    size_t handleJumpIfFalse(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in jump if false");
        }
        Value cond = operandStack.top(); operandStack.pop();
        if (cond.numValue == 0.0) {
            return std::get<int>(instr.operand);
        }
        return pc + 1;
    }

    size_t handleJumpAbsolute(const Bytecode& instr) {
        return std::get<int>(instr.operand);
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
        Value result = callFunction(op.funcName, args);
        operandStack.push(result);
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
            throwRuntimeError("Stack underflow in get iter");
        }
        Value iterable = operandStack.top(); operandStack.pop();
        if (iterable.type != Value::LIST) {
            throwTypeError("Can only iterate over lists");
        }
        operandStack.push(iterable);
    }

    size_t handleForIter(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in for iter");
        }
        Value& iterable = operandStack.top();
        if (iterable.type != Value::LIST) {
            throwTypeError("Can only iterate over lists");
        }
        if (iterable.listValue.empty()) {
            operandStack.pop();
            return std::get<int>(instr.operand);
        }
        Value current = iterable.listValue.back();
        iterable.listValue.pop_back();
        operandStack.push(current);
        return pc + 1;
    }

    void handleReturn() {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in return");
        }
        Value result = operandStack.top();
        operandStack.pop();
        throw ReturnException(result);
    }

    Value callUserFunction(const std::string& name, const std::vector<Value>& args) {
        if (functions.find(name) == functions.end()) {
            throwIdentifierError("Undefined function: " + name);
        }

        FunctionDeclaration* func = functions[name];
        if (func->parameters.size() != args.size()) {
            throwTypeError("Function " + name + " expects " + std::to_string(func->parameters.size()) +
                           " arguments, but got " + std::to_string(args.size()));
        }


        std::map<std::string, Value> savedLocals = localScopes.top();
        localScopes.push({});


        for (size_t i = 0; i < func->parameters.size(); ++i) {
            localScopes.top()[func->parameters[i]] = args[i];
        }


        Value result = Value(Value::NULL_TYPE);
        try {
            CodeGen codegen;
            BytecodeProgram program = codegen.generate(func->body);
            result = execute(program);
        } catch (const ReturnException& e) {
            result = e.value;
        }


        localScopes.pop();
        localScopes.top() = savedLocals;

        return result;
    }


    Value callFunction(const std::string& name, const std::vector<Value>& args) {

        if (localScopes.size() > maxRecursionDepth) {
            throwRecursionError("Maximum recursion depth exceeded");
        }


        if (name == "print") return builtinPrint(args);
        if (name == "input") return builtinInput(args);
        if (name == "len") return builtinLen(args);
        if (name == "type") return builtinType(args);
        if (name == "range") return builtinRange(args);
        if (name == "sleep") return builtinSleep(args);
        if (name == "system") return builtinSystem(args);
        if (name == "exit") return builtinExit(args);
        if (name == "read") return builtinRead(args);
        if (name == "write") return builtinWrite(args);
        if (name == "time") return builtinTime();
        if (name == "list.append") return listAppend(args);


        if (functions.count(name)) {
            return callUserFunction(name, args);
        }

        throwIdentifierError("Undefined function '" + name + "'");
    }


    Value builtinPrint(const std::vector<Value>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            printValue(args[i]);
        }
        std::cout << std::endl;
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
            return Value(static_cast<double>(args[0].strValue.size()));
        }
        if (args[0].type == Value::LIST) {
            return Value(static_cast<double>(args[0].listValue.size()));
        }
        throwTypeError("len() expects string or list");
    }

    Value builtinType(const std::vector<Value>& args) {
        checkArgCount("type", 1, args);
        switch (args[0].type) {
            case Value::NUMBER: return Value("number");
            case Value::STRING: return Value("string");
            case Value::LIST: return Value("list");
            case Value::NULL_TYPE: return Value("null");
            default: return Value("unknown");
        }
    }

    Value builtinRange(const std::vector<Value>& args) {
        checkArgCount("range", 2, args);
        if (args[0].type != Value::NUMBER || args[1].type != Value::NUMBER) {
            throwTypeError("range() expects numbers");
        }

        std::vector<Value> list;
        int start = static_cast<int>(args[0].numValue);
        int end = static_cast<int>(args[1].numValue);
        for (int i = start; i < end; ++i) {
            list.emplace_back(static_cast<double>(i));
        }
        return Value(list);
    }

    Value builtinSleep(const std::vector<Value>& args) {
        checkArgCount("sleep", 1, args);
        if (args[0].type != Value::NUMBER) {
            throwTypeError("sleep() expects a number");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(args[0].numValue)));
        return Value(Value::NULL_TYPE);
    }

    Value builtinSystem(const std::vector<Value>& args) {
        checkArgCount("system", 1, args);
        if (args[0].type != Value::STRING) {
            throwTypeError("system() expects a string");
        }
        int result = std::system(args[0].strValue.c_str());
        return Value(static_cast<double>(result));
    }

    Value builtinExit(const std::vector<Value>& args) {
        checkArgCount("exit", 1, args);
        if (args[0].type != Value::NUMBER) {
            throwTypeError("exit() expects a number");
        }
        std::exit(static_cast<int>(args[0].numValue));
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
        return Value(static_cast<double>(std::time(nullptr)));
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

    void printValue(const Value& value) {
        switch (value.type) {
            case Value::NUMBER: std::cout << value.numValue; break;
            case Value::STRING: std::cout << value.strValue; break;
            case Value::LIST: {
                std::cout << "[";
                for (size_t i = 0; i < value.listValue.size(); ++i) {
                    printValue(value.listValue[i]);
                    if (i < value.listValue.size() - 1) std::cout << ", ";
                }
                std::cout << "]";
                break;
            }
            case Value::NULL_TYPE: std::cout << "null"; break;
        }
    }
};

#endif