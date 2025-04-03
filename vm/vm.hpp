#ifndef VM_HPP
#define VM_HPP

#include "../parser/errors.hpp"
#include "../parser/value.hpp"
#include "../std/std.hpp"
#include "../utils/utils.hpp"
#include "../bytecode/codegen.hpp"
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
    std::map<std::string, Value> consts;

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

        if (op == "+") {
            if (left.type == Value::STRING && right.type == Value::STRING) {
                operandStack.push(Value(left.strValue + right.strValue));
            } else if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                BigNum result = left.bignumValue + right.bignumValue;
                operandStack.push(Value(result));
            } else {
                throwRuntimeError("Cannot add incompatible types");
            }
        } else if (op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op == "|" || op == "&" || op == "~") {
            if (op == "*") {
                if ((left.type == Value::STRING && right.type == Value::NUMBER) || (right.type == Value::STRING && left.type == Value::NUMBER)) {
                    std::string result;
                    auto times = right.bignumValue.get_ll();
                    if (times < 0) throwRuntimeError("Cannot multiply string by negative number");
                    for (long long i = 0; i < times; i++) {
                        result += left.strValue;
                    }
                    operandStack.push(Value(result));
                } else if ((left.type == Value::LIST && right.type == Value::NUMBER) || (right.type == Value::LIST && left.type == Value::NUMBER)) {
                    std::vector<Value> result;
                    auto times = right.bignumValue.get_ll();
                    if (times < 0) throwRuntimeError("Cannot multiply list by negative number");
                    for (long long i = 0; i < times; i++) {
                        result.insert(result.end(), left.listValue.begin(), left.listValue.end());
                    }
                    operandStack.push(Value(result));
                } else if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                    operandStack.push(Value(left.bignumValue * right.bignumValue));
                } else {
                    throwRuntimeError("Invalid operand types for multiplication");
                }
            } else if (left.type != Value::NUMBER || right.type != Value::NUMBER) {
                throwRuntimeError("Operator " + op + " requires numbers");
            } else {
                BigNum result;
                if (op == "-") result = left.bignumValue - right.bignumValue;
                else if (op == "/") result = left.bignumValue / right.bignumValue;
                else if (op == "%") result = left.bignumValue % right.bignumValue;
                else if (op == "^") result = left.bignumValue.pow(right.bignumValue);
                else if (op == "|") result = left.bignumValue.get_ll() | right.bignumValue.get_ll();
                else if (op == "&") result = left.bignumValue.get_ll() & right.bignumValue.get_ll();
                else if (op == "~") result = ~right.bignumValue.get_ll();
                operandStack.push(Value(result));
            }
        } else if (op == "<" || op == "<=" || op == "==" ||
                  op == "!=" || op == ">" || op == ">=") {
            auto handle_compare = [&](auto cmp) {
                bool result = cmp(left, right);
                operandStack.push(Value(BigNum(result ? 1 : 0)));
            };
            if (left.type == Value::STRING && right.type == Value::STRING) {
                if (op == "<") handle_compare([](auto& l, auto& r){ return l.strValue < r.strValue; });
                else if (op == ">") handle_compare([](auto& l, auto& r){ return l.strValue > r.strValue; });
                else if (op == "<=") handle_compare([](auto& l, auto& r){ return l.strValue <= r.strValue; });
                else if (op == ">=") handle_compare([](auto& l, auto& r){ return l.strValue >= r.strValue; });
                else if (op == "==") handle_compare([](auto& l, auto& r){ return l.strValue == r.strValue; });
                else if (op == "!=") handle_compare([](auto& l, auto& r){ return l.strValue != r.strValue; });
            } else {
                if (op == "<") handle_compare([](auto& l, auto& r){ return l.bignumValue < r.bignumValue; });
                else if (op == ">") handle_compare([](auto& l, auto& r){ return l.bignumValue > r.bignumValue; });
                else if (op == "<=") handle_compare([](auto& l, auto& r){ return l.bignumValue <= r.bignumValue; });
                else if (op == ">=") handle_compare([](auto& l, auto& r){ return l.bignumValue >= r.bignumValue; });
                else if (op == "==") handle_compare([](auto& l, auto& r){ return l.bignumValue == r.bignumValue; });
                else if (op == "!=") handle_compare([](auto& l, auto& r){ return l.bignumValue != r.bignumValue; });
            }
        } else if (op == "and" || op == "or") {
            bool leftValue = left.bignumValue != 0;
            bool rightValue = right.bignumValue != 0;
            bool result;
            if (op == "and") {
                result = leftValue && rightValue;
            } else { // op == "or"
                result = leftValue || rightValue;
            }
            operandStack.push(Value(BigNum(result ? 1 : 0)));
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
            if (operandStack.empty()) {
                throwRuntimeError("Stack underflow in function call");
            }
            args.insert(args.begin(), operandStack.top());
            operandStack.pop();
        }

        Value result;
        bool isMethodCall = false;


        if (args.size() > 1 && args[0].type == Value::OBJECT && args[1].type == Value::STRING) {
            FunctionDeclaration* method;
            std::string className = args[1].strValue;
            if (args[0].functions.count(op.funcName)) method = args[0].functions[op.funcName];
            else throwIdentifierError("Undefined method: " + className + "." + op.funcName);
            
            BytecodeProgram DefaultValuesBytecodes;
            CodeGen DefaultVal(std::map<std::string, ClassDeclaration*>(), consts, functions);

            for (size_t i = op.argCount - 2; i < method->parameters.size(); i++) {
                if (method->default_values[i] != nullptr) {
                    DefaultVal.genExpr(method->default_values[i], DefaultValuesBytecodes);
                } else {
                    throwSyntaxError("Missing argument for parameter '" + method->parameters[i] + "'");
                }
            }

            if (!DefaultValuesBytecodes.empty()) {
                Frame defaultValFrame(DefaultValuesBytecodes, &frames.top());
                frames.push(defaultValFrame);
                execute();
                
                for (size_t i = op.argCount - 2; i < method->parameters.size(); i++) {
                    if (!operandStack.empty()) {
                        args.push_back(operandStack.top());
                        operandStack.pop();
                    }
                }
                frames.pop();
            }
            Value &self = args[0];

            // for (auto function : self.functions) {
            //     printf("%s ", function.first.c_str()); 
            // }
            // printf("\n");
            
            if (self.functions.count(op.funcName)) {
                Frame newFrame(method->bytecode, &frames.top());
                
                newFrame.locals["self"] = self;
                
                for (size_t i = 0; i < method->parameters.size(); ++i) {
                    if (i + 2 < args.size()) {
                        newFrame.locals[method->parameters[i]] = args[i + 2];
                    }
                }
                frames.push(newFrame);

                result = execute();

                std::string fullName = className;
                size_t dotPos = fullName.find('.');
                Value* classPtr = &currFrame.locals[className];
                if (dotPos != std::string::npos) {
                    std::string firstVar = fullName.substr(0, dotPos);
                    classPtr = &currFrame.locals[firstVar];
                    size_t prevDotPos = dotPos;
                    while ((dotPos = fullName.find('.', prevDotPos + 1)) != std::string::npos) {
                        std::string memberName = fullName.substr(prevDotPos + 1, dotPos - prevDotPos - 1);
                        classPtr = &(classPtr->objectMembers[memberName]);
                        prevDotPos = dotPos;
                    }
                    classPtr = &(classPtr->objectMembers[fullName.substr(prevDotPos + 1)]);
                }

                for (const auto& it : frames.top().locals["self"].objectMembers) {
                    classPtr->objectMembers[it.first] = it.second;
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
            return Value();
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