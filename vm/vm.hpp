#ifndef VM_HPP
#define VM_HPP

#include "../utils/core.hpp"
#include "../std/std.hpp"
#include <vector>
#include <map>
#include <stack>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <memory>

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

    Value execute() {
        if (frames.empty()) {
            return Value();
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
    void handleLoadConst(const Bytecode& instr) {
        if (auto ival = std::get_if<BigNum>(&instr.operand)) {
            operandStack.push(Value(*ival));
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
        if (index.type != Value::NUMBER) throwTypeError("Index must be number");
        BigNum idx = index.bignumValue;
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        operandStack.push(list.listValue[idx.get_ll()]);
    }

    void handleStoreSubscript() {
        if (operandStack.size() < 3) throwRuntimeError("Stack underflow");
        Value value = operandStack.top(); operandStack.pop();
        Value index = operandStack.top(); operandStack.pop();
        Value list = operandStack.top(); operandStack.pop();
        if (list.type != Value::LIST) throwTypeError("Expected list");
        if (index.type != Value::NUMBER) throwTypeError("Index must be number");
        BigNum idx = index.bignumValue;
        if (idx < 0 || idx >= list.listValue.size()) throwIndexError("Index out of range");
        list.listValue[idx.get_ll()] = value;
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

        if (op == "+" || op == "-" || op == "*" || op == "/") {
            BigNum result;
            if (op == "+") result = left.bignumValue + right.bignumValue;
            else if (op == "-") result = left.bignumValue - right.bignumValue;
            else if (op == "*") result = left.bignumValue * right.bignumValue;
            else if (op == "/") result = left.bignumValue / right.bignumValue;
            else {
                throwRuntimeError("Unknown binary operator: " + op);
            }
            operandStack.push(Value(result));
        } else if (op == "<" || op == "<=" || op == "==" ||
                  op == "!=" || op == ">" || op == ">=") {
            auto handle_compare = [&](auto cmp) {
                bool result = cmp(left, right);
                operandStack.push(Value(BigNum(result ? 1 : 0)));
            };
            if (op == "<") handle_compare([](auto& l, auto& r){ return l.bignumValue < r.bignumValue; });
            else if (op == ">") handle_compare([](auto& l, auto& r){ return l.bignumValue > r.bignumValue; });
            else if (op == "<=") handle_compare([](auto& l, auto& r){ return l.bignumValue <= r.bignumValue; });
            else if (op == ">=") handle_compare([](auto& l, auto& r){ return l.bignumValue >= r.bignumValue; });
            else if (op == "==") handle_compare([](auto& l, auto& r){ return l.bignumValue == r.bignumValue; });
            else if (op == "!=") handle_compare([](auto& l, auto& r){ return l.bignumValue != r.bignumValue; });
        } else if (op == "[]") {
            if (left.type != Value::LIST) {
                throwTypeError("Expected list for [] operator");
            }
            if (right.type != Value::NUMBER) {
                throwTypeError("Expected number for list index");
            }
            BigNum index = right.bignumValue;
            if (index < 0 || index >= left.listValue.size()) {
                throwIndexError("List index out of range");
            }
            operandStack.push(left.listValue[index.get_ll()]);
        } else {
            throwRuntimeError("Unknown operator: " + op);
        }
    }

    size_t handleJump(const Bytecode& instr) {
        return (std::get<BigNum>(instr.operand)).get_ll();
    }

    size_t handleJumpIfFalse(const Bytecode& instr, size_t pc) {
        if (operandStack.empty()) {
            throwRuntimeError("Stack underflow in jump if false");
        }
        Value cond = operandStack.top(); operandStack.pop();
        if (cond.bignumValue == 0) {
            return (std::get<BigNum>(instr.operand)).get_ll();
        }
        return pc + 1;
    }

    void handleBuildList(const Bytecode& instr) {
        long long count = (std::get<BigNum>(instr.operand)).get_ll();
        if (BigNum(operandStack.size()) < count) {
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
            return (std::get<BigNum>(instr.operand)).get_ll();
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

    Value callBuiltinFunction(const std::string& name, std::vector<Value> args) {
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
        } else if (name == "append") {
            return listAppend(args);
        } else if (name == "erase") {
            return listErase(args);
        } else if (name == "insert") {
            return listInsert(args);
        } else {
            throwIdentifierError("Undefined builtin function: " + name);
        }
    }

    void handleReturn(Frame& frame) {
        if (!operandStack.empty()) {
            frame.returnValue = operandStack.top();
            operandStack.pop();
        } else {
            frame.returnValue = Value();
        }
        frame.pc = frame.program.size();
    }
};

#endif