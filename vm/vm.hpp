#ifndef VM_HPP
#define VM_HPP

#include "../parser/errors.hpp"
#include "../parser/value.hpp"
#include "../std/std.hpp"
#include "../utils/utils.hpp"
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

    void printFrameStack() {
        std::stack<Frame> tempFrames = frames;
        int frameIndex = tempFrames.size() - 1;

        while (!tempFrames.empty()) {
            const Frame& frame = tempFrames.top();
            std::cout << "Frame " << frameIndex << ":" << std::endl;


            std::cout << "  Locals:" << std::endl;
            if (frame.locals.empty()) {
                std::cout << "    <empty>" << std::endl;
            } else {
                for (const auto& pair : frame.locals) {
                    std::cout << "    " << pair.first << std::endl;

                }
            }


            std::cout << "  Parent Frame: ";
            if (frame.parent == nullptr) {
                std::cout << "<none>" << std::endl;
            } else {

                std::cout << "<parent>" << std::endl;
            }


            std::cout << "  Program:" << std::endl;
            for (size_t i = 0; i < frame.program.size(); ++i) {
                const Bytecode& instr = frame.program[i];
                std::cout << "    " << std::setw(4) << i << ": ";
                switch (instr.op) {
                    case LOAD_CONST: std::cout << "LOAD_CONST"; break;
                    case LOAD_VAR: std::cout << "LOAD_VAR"; break;
                    case STORE_VAR: std::cout << "STORE_VAR"; break;
                    case BINARY_OP: std::cout << "BINARY_OP"; break;
                    case JUMP: std::cout << "JUMP"; break;
                    case JUMP_IF_FALSE: std::cout << "JUMP_IF_FALSE"; break;
                    case CALL_FUNCTION: std::cout << "CALL_FUNCTION"; break;
                    case BUILD_LIST: std::cout << "BUILD_LIST"; break;
                    case POP: std::cout << "POP"; break;
                    case RETURN: std::cout << "RETURN"; break;
                    case LOAD_SUBSCRIPT: std::cout << "LOAD_SUBSCRIPT"; break;
                    case STORE_SUBSCRIPT: std::cout << "STORE_SUBSCRIPT"; break;
                    case CREATE_OBJECT: std::cout << "CREATE_OBJECT"; break;
                    case LOAD_MEMBER: std::cout << "LOAD_MEMBER"; break;
                    case STORE_MEMBER: std::cout << "STORE_MEMBER"; break;
                    case LOAD_FUNC: std::cout << "LOAD_FUNC"; break;
                    case STORE_MEMBER_FUNC: std::cout << "STORE_MEMBER_FUNC"; break;
                    // case CLEAR: std::cout << "CLEAR"; break;
                    default: std::cout << "Unknown opcode"; break;
                }
                try{
                    if (!std::get<std::string>(instr.operand).empty()) {
                        std::cout << " " << std::get<std::string>(instr.operand);
                    }
                } catch(...) {

                }

                try{
                    if (std::get<BigNum>(instr.operand)!= 0) {
                        std::cout << " " << std::get<BigNum>(instr.operand).get_ll();
                    }
                } catch(...) {

                }

                try{
                    if (!std::get<CallFunctionOperand>(instr.operand).funcName.empty()) {
                        std::cout << " " << std::get<CallFunctionOperand>(instr.operand).funcName;
                    }
                } catch(...) {

                }

                std::cout << std::endl;
            }


            std::cout << "  PC: " << frame.pc << std::endl;

            tempFrames.pop();
            frameIndex--;
        }

        std::cout << "Operand Stack:" << std::endl;
        std::stack<Value> tempOperandStack = operandStack;
        if (tempOperandStack.empty()) {
            std::cout << "  <empty>" << std::endl;
        } else {
            std::vector<Value> stackValues;
            while (!tempOperandStack.empty()) {
                stackValues.push_back(tempOperandStack.top());
                tempOperandStack.pop();
            }
            for (auto it = stackValues.rbegin(); it != stackValues.rend(); ++it) {
                std::cout << "  "; printValue(*it); cout << std::endl;
            }
        }
    }

    Value execute() {
        if (frames.empty()) {
            return Value();
        }

        Frame& currentFrame = frames.top();

        while (currentFrame.pc < currentFrame.program.size()) {
            const Bytecode& instr = currentFrame.program[currentFrame.pc];

        //    printFrameStack();

            try {
                switch (instr.op) {
                    case LOAD_CONST: handleLoadConst(instr); break;
                    case LOAD_VAR: handleLoadVar(instr, currentFrame); break;
                    case STORE_VAR: handleStoreVar(instr, currentFrame); break;
                    case BINARY_OP: handleBinaryOp(instr); break;
                    case JUMP: currentFrame.pc = handleJump(instr); continue;
                    case JUMP_IF_FALSE: currentFrame.pc = handleJumpIfFalse(instr, currentFrame.pc); continue;
                    case CALL_FUNCTION: handleCallFunction(instr, currentFrame); break;
                    case BUILD_LIST: handleBuildList(instr); break;
                    case POP: operandStack.pop(); break;
                    case RETURN: handleReturn(currentFrame); break;
                    case LOAD_SUBSCRIPT: handleLoadSubscript(); break;
                    case STORE_SUBSCRIPT: handleStoreSubscript(); break;
                    case CREATE_OBJECT: {
                        Value obj;
                        obj.type = Value::OBJECT;
                        obj.objectMembers = {};
                        operandStack.push(obj);
                        break;
                    }
                    case LOAD_FUNC: {
                        std::string funcName = std::get<std::string>(instr.operand);
                        if (functions.count(funcName)) {
                            Value funcValue;
                            funcValue.type = Value::OBJECT;
                            funcValue.functions[funcName] = functions[funcName];
                            operandStack.push(funcValue);
                        } else {
                            throwRuntimeError("Function not found: " + funcName);
                        }
                        break;
                    }
                    case STORE_MEMBER_FUNC: {
                        if (operandStack.size() < 3) throwRuntimeError("Stack underflow");
                        Value func = operandStack.top(); operandStack.pop();
                        Value methodName = operandStack.top(); operandStack.pop();
                        Value obj = operandStack.top(); operandStack.pop();

                        if (obj.type != Value::OBJECT) throwTypeError("Cannot store method on non-object");
                        if (methodName.type != Value::STRING) throwTypeError("Method name must be a string");


                        obj.functions[methodName.strValue] = functions[func.functions.begin()->first];
                        operandStack.push(obj);
                        break;
                    }
                    case STORE_MEMBER: {
                        std::string member = std::get<std::string>(instr.operand);
                        if (operandStack.size() < 2) {
                            throwRuntimeError("Stack underflow in STORE_MEMBER");
                        }
                        Value obj = operandStack.top();
                        operandStack.pop();
                        Value value = operandStack.top();
                        operandStack.pop();
                        if (obj.type != Value::OBJECT) {
                            throwTypeError("Cannot store member on non-object");
                        }
                        obj.objectMembers[member] = value;
                        operandStack.push(obj);
                        break;
                    }
                    case LOAD_MEMBER: {
                        std::string member = std::get<std::string>(instr.operand);
                        Value obj = operandStack.top();
                        operandStack.pop();

                        if (obj.type != Value::OBJECT) {
                            throwTypeError("Cannot access member of non-object");
                        }
                        if (obj.objectMembers.find(member) == obj.objectMembers.end()) {
                            throwIdentifierError("Undefined member: " + member);
                        }
                        operandStack.push(obj.objectMembers[member]);
                        break;
                    }
                    // case CLEAR: {
                    //     std::stack<Value>().swap(operandStack);
                    //     break;
                    // }
                    case LABEL: break;
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
            operandStack.push(Value());
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

        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^") {
            BigNum result;
            if (op == "+") result = left.bignumValue + right.bignumValue;
            else if (op == "-") result = left.bignumValue - right.bignumValue;
            else if (op == "*") result = left.bignumValue * right.bignumValue;
            else if (op == "/") result = left.bignumValue / right.bignumValue;
            else if (op == "%") result = left.bignumValue % right.bignumValue;
            else if (op == "^") result = left.bignumValue.pow(right.bignumValue);
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

    void handleCallFunction(const Bytecode& instr, Frame& currFrame) {
        CallFunctionOperand op = std::get<CallFunctionOperand>(instr.operand);
        std::vector<Value> args;


        for (int i = 0; i < op.argCount; i++) {
            args.insert(args.begin(), operandStack.top());
            operandStack.pop();
        }

        Value result;
        bool isMethodCall = false;


        if (args.size() > 1 && args[0].type == Value::OBJECT && args[1].type == Value::STRING) {
            Value& self = args[0];
            std::string className = args[1].strValue;
            if (self.functions.count(op.funcName)) {
                FunctionDeclaration* method = self.functions[op.funcName];

                Frame newFrame(method->bytecode, &frames.top());

                newFrame.locals["self"] = self;

                for (size_t i = 0; i < method->parameters.size(); ++i) {
                    if (i + 2 < args.size()) {
                        newFrame.locals[method->parameters[i]] = args[i + 2];
                    }
                }
                frames.push(newFrame);
                result = execute();

                for (const auto& it : frames.top().locals["self"].objectMembers) {
                    currFrame.locals[className].objectMembers[it.first] = it.second;
                }

                frames.pop();

                isMethodCall = true;
            } else {
                throwIdentifierError("Undefined method: " + className + "." + op.funcName);
            }
        }


        if (!isMethodCall && functions.count(op.funcName)) {
            FunctionDeclaration* func = functions[op.funcName];
            Frame newFrame(func->bytecode, &frames.top());
            for (size_t i = 0; i < func->parameters.size(); ++i) {
                if (i < args.size()) {
                    newFrame.locals[func->parameters[i]] = args[i];
                }
            }
            frames.push(newFrame);
            result = execute();
            frames.pop();
        } else if (!isMethodCall) {
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
        } else if (name == "floor") {
            return builtinMathFloor(args);
        } else if (name == "ceil") {
            return builtinMathCeil(args);
        } else if (name == "abs") {
            return builtinMathAbs(args);
        } else if (name == "pow") {
            return builtinMathPow(args);
        } else if (name == "round") {
            return builtinMathRound(args);
        } else if (name == "sqrt") {
            return builtinMathSqrt(args);
        } else if (name == "list") {
            return builtinList(args);
        } else if (name == "str") {
            return builtinStr(args);
        } else if (name == "number") {
            return builtinNumber(args);
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