#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "core.hpp"
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stack>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <string>

class Interpreter {
public:
    void interpret(const std::vector<Statement*>& statements, const size_t DepthLimit = 65536) {
        maxDepth = DepthLimit;
        for (Statement* statement : statements) {
            execute(statement, 0);
        }
    }

private:
    std::map<std::string, Value> variables;
    std::map<std::string, FunctionDeclaration*> functions;
    size_t maxDepth;

    void execute(Statement* statement, int depth = 0) {
        std::vector<std::pair<Statement*, int>> stack;
        stack.push_back({statement, depth});

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();
            Statement* stmt = current.first;
            int currentDepth = current.second;

            if (currentDepth > maxDepth) {
                throw std::runtime_error("Recursion depth limit exceeded");
            }

            try {
                if (auto block = dynamic_cast<BlockStatement*>(stmt)) {
                    for (auto it = block->statements.rbegin(); it != block->statements.rend(); ++it) {
                        stack.push_back({*it, currentDepth + 1});
                    }
                } else if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
                    Value value;
                    if (varDecl->initializer != nullptr) {
                        value = evaluate(varDecl->initializer, currentDepth + 1);
                    } else {
                        value = Value(Value::NULL_TYPE);
                    }
                    variables[varDecl->name] = value;
                } else if (auto assignment = dynamic_cast<Assignment*>(stmt)) {
                    if (assignment->isSubscriptAssignment) {
                        if (variables.find(assignment->target) == variables.end()) {
                            throw std::runtime_error("Undefined variable: " + assignment->target);
                        }
                        Value& listValue = variables[assignment->target];
                        if (listValue.type != Value::LIST) {
                            throw std::runtime_error("Target is not a list");
                        }
                        Value indexValue = evaluate(assignment->index, currentDepth + 1);
                        if (indexValue.type != Value::NUMBER) {
                            throw std::runtime_error("Index must be a number");
                        }
                        int index = static_cast<int>(indexValue.numValue);
                        if (index < 0 || index >= listValue.listValue.size()) {
                            throw std::runtime_error("Index out of range");
                        }
                        Value value = evaluate(assignment->value, currentDepth + 1);
                        listValue.listValue[index] = value;
                    } else {
                        Value value = evaluate(assignment->value, currentDepth + 1);
                        variables[assignment->target] = value;
                    }
                } else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt)) {
                    functions[funcDecl->name] = funcDecl;
                } else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
                    while (true) {
                        Value conditionValue = evaluate(whileStmt->condition, currentDepth + 1);
                        if (conditionValue.numValue == 0) {
                            break; 
                        }
                        for (Statement* bodyStmt : whileStmt->body) {
                            try {
                                execute(bodyStmt, currentDepth + 1);
                            } catch (ContinueException& e) {
                                break; 
                            } catch (BreakException& e) {
                                return; 
                            }
                        }
                    }
                } else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
                    if (evaluate(ifStmt->condition, currentDepth + 1).numValue != 0) {
                        for (auto it = ifStmt->body.rbegin(); it != ifStmt->body.rend(); ++it) {
                            stack.push_back({*it, currentDepth + 1});
                        }
                    } else if (!ifStmt->elseBody.empty()) {
                        for (auto it = ifStmt->elseBody.rbegin(); it != ifStmt->elseBody.rend(); ++it) {
                            stack.push_back({*it, currentDepth + 1});
                        }
                    }
                } else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
                    Value value = evaluate(returnStmt->value, currentDepth + 1);
                    throw ReturnException(value);
                } else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
                    evaluate(exprStmt->expression, currentDepth + 1);
                } else if (dynamic_cast<BreakStatement*>(stmt)) {
                    throw BreakException();
                } else if (dynamic_cast<ContinueStatement*>(stmt)) {
                    throw ContinueException();
                } else {
                    throw std::runtime_error("Unknown statement type");
                }
            } catch (ReturnException& e) {
                throw;
            }
        }
    }

    Value evaluate(Expression* expr, int depth) {
        if (depth > maxDepth) {
            throw std::runtime_error("Recursion depth limit exceeded");
        }

        std::vector<std::pair<Expression*, int>> stack;
        std::vector<Value> valueStack;
        stack.push_back({expr, depth});

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();
            Expression* currentExpr = current.first;
            int currentDepth = current.second;

            try {
                if (auto binExpr = dynamic_cast<BinaryExpression *>(currentExpr)) {
                    if (binExpr->op == "not") {
                        stack.push_back({binExpr->left, currentDepth + 1});
                    } else {
                        if (valueStack.size() < 2) {
                            stack.push_back({binExpr, currentDepth});
                            stack.push_back({binExpr->right, currentDepth + 1});
                            stack.push_back({binExpr->left, currentDepth + 1});
                        } else {
                            Value right = valueStack.back();
                            valueStack.pop_back();
                            Value left = valueStack.back();
                            valueStack.pop_back();

                            if (binExpr->op == "+") {
                                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue + right.numValue));
                                } else if (left.type == Value::STRING && right.type == Value::STRING) {
                                    valueStack.push_back(Value(left.strValue + right.strValue));
                                } else {
                                    throw std::runtime_error("Type mismatch in binary expression");
                                }
                            } else if (binExpr->op == "-") {
                                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue - right.numValue));
                                } else {
                                    throw std::runtime_error("Type mismatch in binary expression");
                                }
                            } else if (binExpr->op == "*") {
                                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue * right.numValue));
                                } else {
                                    throw std::runtime_error("Type mismatch in binary expression");
                                }
                            } else if (binExpr->op == "/") {
                                if (left.type == Value::NUMBER && right.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue / right.numValue));
                                } else {
                                    throw std::runtime_error("Type mismatch in binary expression");
                                }
                            } else if (binExpr->op == "<") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue < right.numValue ? 1.0 : 0.0));
                                }
                            } else if (binExpr->op == ">") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue > right.numValue ? 1.0 : 0.0));
                                }
                            } else if (binExpr->op == "<=") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue <= right.numValue ? 1.0 : 0.0));
                                }
                            } else if (binExpr->op == ">=") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue >= right.numValue ? 1.0 : 0.0));
                                }
                            } else if (binExpr->op == "==") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue == right.numValue ? 1.0 : 0.0));
                                } else if (left.type == Value::STRING) {
                                    valueStack.push_back(Value(left.strValue == right.strValue ? 1.0 : 0.0));
                                }
                            } else if (binExpr->op == "!=") {
                                if (left.type == Value::NUMBER) {
                                    valueStack.push_back(Value(left.numValue != right.numValue ? 1.0 : 0.0));
                                } else if (left.type == Value::STRING) {
                                    valueStack.push_back(Value(left.strValue != right.strValue ? 1.0 : 0.0));
                                }
                            } else {
                                throw std::runtime_error("Unknown operator: " + binExpr->op);
                            }
                        }
                    }
                } else if (auto numLit = dynamic_cast<NumberLiteral *>(currentExpr)) {
                    valueStack.push_back(Value(numLit->value));
                } else if (auto strLit = dynamic_cast<StringLiteral *>(currentExpr)) {
                    valueStack.push_back(Value(strLit->value));
                } else if (auto nullLit = dynamic_cast<NullLiteral*>(currentExpr)) {
                    valueStack.push_back(Value(Value::NULL_TYPE)); 
                } else if (auto id = dynamic_cast<Identifier *>(currentExpr)) {
                    if (variables.find(id->name) == variables.end()) {
                        throw std::runtime_error("Undefined variable: " + id->name);
                    }
                    valueStack.push_back(variables[id->name]);
                } else if (auto funcCall = dynamic_cast<FunctionCall *>(currentExpr)) {
                    std::string funcName = funcCall->name;
                    if (funcName.find(".") != std::string::npos) {
                        size_t dotPos = funcName.find(".");
                        std::string listName = funcName.substr(0, dotPos);
                        std::string memberFunc = funcName.substr(dotPos + 1);

                        if (variables.find(listName) == variables.end()) {
                            throw std::runtime_error("Undefined variable: " + listName);
                        }
                        Value listValue = variables[listName];
                        if (listValue.type != Value::LIST) {
                            throw std::runtime_error("Variable is not a list: " + listName);
                        }

                        if (memberFunc == "size") {
                            if (funcCall->arguments.size() != 0) {
                                throw std::runtime_error("size function expects no arguments");
                            }
                            valueStack.push_back(Value(static_cast<double>(listValue.listValue.size())));
                        } else if (memberFunc == "empty") {
                            if (funcCall->arguments.size() != 0) {
                                throw std::runtime_error("empty function expects no arguments");
                            }
                            valueStack.push_back(Value(listValue.listValue.empty() ? 1.0 : 0.0));
                        } else if (memberFunc == "append") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("append function expects exactly one argument");
                            }
                            Value appendValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            listValue.listValue.push_back(appendValue);
                            valueStack.push_back(listValue);
                        } else if (memberFunc == "insert") {
                            if (funcCall->arguments.size() != 2) {
                                throw std::runtime_error("insert function expects exactly two arguments");
                            }
                            Value posValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (posValue.type != Value::NUMBER) {
                                throw std::runtime_error("First argument to insert must be a number");
                            }
                            int pos = static_cast<int>(posValue.numValue);
                            if (pos < 0 || pos > listValue.listValue.size()) {
                                throw std::runtime_error("Position out of range in insert function");
                            }
                            Value insertValue = evaluate(funcCall->arguments[1], currentDepth + 1);
                            listValue.listValue.insert(listValue.listValue.begin() + pos, insertValue);
                            valueStack.push_back(listValue);
                        } else if (memberFunc == "erase") {
                            if (funcCall->arguments.size() != 2) {
                                throw std::runtime_error("erase function expects exactly two arguments");
                            }
                            Value beginValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (beginValue.type != Value::NUMBER) {
                                throw std::runtime_error("First argument to erase must be a number");
                            }
                            int begin = static_cast<int>(beginValue.numValue);
                            Value endValue = evaluate(funcCall->arguments[1], currentDepth + 1);
                            if (endValue.type != Value::NUMBER) {
                                throw std::runtime_error("Second argument to erase must be a number");
                            }
                            int end = static_cast<int>(endValue.numValue);
                            if (begin < 0 || end > listValue.listValue.size() || begin > end) {
                                throw std::runtime_error("Invalid range in erase function");
                            }
                            listValue.listValue.erase(listValue.listValue.begin() + begin,
                                                      listValue.listValue.begin() + end);
                            valueStack.push_back(listValue);
                        } else {
                            throw std::runtime_error("Undefined member function: " + memberFunc);
                        }
                    } else {
                        if (funcName == "type") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("type function expects exactly one argument");
                            }
                            Value value = evaluate(funcCall->arguments[0], currentDepth + 1);
                            std::string typeStr;
                            switch (value.type) {
                                case Value::NUMBER: typeStr = "number"; break;
                                case Value::STRING: typeStr = "string"; break;
                                case Value::LIST: typeStr = "list"; break;
                                case Value::NULL_TYPE: typeStr = "null"; break;
                                default: typeStr = "unknown"; break;
                            }
                            valueStack.push_back(Value(typeStr));
                        } else if (funcName == "len") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("len function expects exactly one argument");
                            }
                            Value argValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (argValue.type == Value::STRING) {
                                const std::string& str = argValue.strValue;
                                valueStack.push_back(Value(static_cast<double>(utf8len(str))));
                            } else if (argValue.type == Value::LIST) {
                                valueStack.push_back(Value(static_cast<double>(argValue.listValue.size())));
                            } else {
                                throw std::runtime_error("len function expects a string or list as argument");
                            }
                        } else if (funcCall->name == "input") {
                            if (funcCall->arguments.size() == 1) {
                                Value prompt = evaluate(funcCall->arguments[0], currentDepth + 1);
                                if (prompt.type == Value::STRING) {
                                    std::cout << prompt.strValue;
                                } else if (prompt.type == Value::NUMBER) {
                                    std::cout << prompt.numValue;
                                }
                            }
                            std::string inputStr;
                            std::getline(std::cin, inputStr);
                            try {
                                double numValue = std::stod(inputStr);
                                valueStack.push_back(Value(numValue));
                            } catch (const std::invalid_argument &) {
                                valueStack.push_back(Value(inputStr));
                            }
                        } else if (funcCall->name == "print") {
                            for (Expression *arg: funcCall->arguments) {
                                Value value = evaluate(arg, currentDepth + 1);
                                printValue(value);
                            }
                            std::cout << std::endl;
                            valueStack.push_back(Value(0.0));
                        } else if (funcCall->name == "time") {
                            if (funcCall->arguments.size() != 0) {
                                throw std::runtime_error("time function expects no arguments");
                            }
                            valueStack.push_back(Value(static_cast<double>(std::time(nullptr))));
                        } else if (funcName == "range") {
                            if (funcCall->arguments.size() != 2) {
                                throw std::runtime_error("range function expects exactly two arguments");
                            }
                            Value beginValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            Value endValue = evaluate(funcCall->arguments[1], currentDepth + 1);
                            if (beginValue.type != Value::NUMBER || endValue.type != Value::NUMBER) {
                                throw std::runtime_error("range function expects numbers as arguments");
                            }
                            int begin = static_cast<int>(beginValue.numValue);
                            int end = static_cast<int>(endValue.numValue);
                            std::vector<Value> list;
                            for (int i = begin; i < end; ++i) {
                                list.push_back(Value(static_cast<double>(i)));
                            }
                            valueStack.push_back(Value(list));
                        } else if (funcCall->name == "sleep") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("sleep function expects exactly one argument");
                            }
                            Value msValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (msValue.type != Value::NUMBER) {
                                throw std::runtime_error("sleep function expects a number as argument");
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(msValue.numValue)));
                            valueStack.push_back(Value(0.0));
                        } else if (funcCall->name == "system") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("system function expects exactly one argument");
                            }
                            Value commandValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (commandValue.type != Value::STRING) {
                                throw std::runtime_error("system function expects a string as argument");
                            }
                            int result = std::system(commandValue.strValue.c_str());
                            valueStack.push_back(Value(static_cast<double>(result)));
                        } else if (funcCall->name == "exit") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("exit function expects exactly one argument");
                            }
                            Value exitValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (exitValue.type != Value::NUMBER) {
                                throw std::runtime_error("exit function expects a number as argument");
                            }
                            std::exit(static_cast<int>(exitValue.numValue));
                        } else if (funcCall->name == "read") {
                            if (funcCall->arguments.size() != 1) {
                                throw std::runtime_error("read function expects exactly one argument");
                            }
                            Value filenameValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            if (filenameValue.type != Value::STRING) {
                                throw std::runtime_error("read function expects a string as argument");
                            }
                            std::ifstream file(filenameValue.strValue);
                            if (!file.is_open()) {
                                throw std::runtime_error("Could not open file: " + filenameValue.strValue);
                            }
                            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            valueStack.push_back(Value(content));
                        } else if (funcCall->name == "write") {
                            if (funcCall->arguments.size() != 2) {
                                throw std::runtime_error("write function expects exactly two arguments");
                            }
                            Value filenameValue = evaluate(funcCall->arguments[0], currentDepth + 1);
                            Value contentValue = evaluate(funcCall->arguments[1], currentDepth + 1);
                            if (filenameValue.type != Value::STRING || contentValue.type != Value::STRING) {
                                throw std::runtime_error("write function expects strings as arguments");
                            }
                            std::ofstream file(filenameValue.strValue);
                            if (!file.is_open()) {
                                throw std::runtime_error("Could not open file: " + filenameValue.strValue);
                            }
                            file << contentValue.strValue;
                            valueStack.push_back(Value(0.0));
                        } else if (functions.find(funcCall->name) != functions.end()) {
                            FunctionDeclaration *func = functions[funcName];
                            if (funcCall->arguments.size() != func->parameters.size()) {
                                std::cout << evaluate(funcCall->arguments[1], currentDepth + 1).numValue << "\n";
                                std::cout << "Function " << funcName << " expects " << func->parameters.size()
                                          << " arguments, but got " << funcCall->arguments.size() << std::endl;
                                throw std::runtime_error("Incorrect number of arguments for function " + funcName);
                            }

                            if (currentDepth + 1 > maxDepth) {
                                throw std::runtime_error("Recursion depth limit exceeded");
                            }

                            std::map<std::string, Value> savedVariables = variables;
                            for (size_t i = 0; i < func->parameters.size(); ++i) {
                                std::string param = func->parameters[i];
                                Value argValue = evaluate(funcCall->arguments[i], currentDepth + 1);
                                variables[param] = argValue;
                            }
                            Value result = Value(0.0);
                            for (Statement *stmt: func->body) {
                                try {
                                    execute(stmt, currentDepth + 1);
                                } catch (ReturnException &e) {
                                    result = e.value;
                                    break;
                                }
                            }
                            variables = savedVariables;
                            valueStack.push_back(result);
                        } else if (funcCall->name == "import") {
                            for (Expression *arg: funcCall->arguments) {
                                Value filenameValue = evaluate(arg, currentDepth + 1);
                                if (filenameValue.type != Value::STRING) {
                                    throw std::runtime_error("import function expects a string as argument");
                                }

                                std::ifstream inputFile(filenameValue.strValue);
                                if (!inputFile.is_open()) {
                                    throw std::runtime_error(
                                            "Can't open file \"" + filenameValue.strValue + "\" to run.");
                                }

                                std::string command, commands;
                                while (std::getline(inputFile, command)) {
                                    commands += command + "\n";
                                }
                                inputFile.close();

                                Lexer lexer(commands);
                                std::vector<Token> tokens = lexer.tokenize();

                                Parser parser(tokens);
                                std::vector<Statement *> statements = parser.parse();

                                for (Statement *statement: statements) {
                                    execute(statement);
                                }
                            }
                            valueStack.push_back(Value(0.0));
                        } else {
                            throw std::runtime_error("Undefined function: " + funcCall->name);
                        }
                    }
                } else if (auto listLit = dynamic_cast<ListLiteral *>(currentExpr)) {
                    std::vector<Value> elements;
                    for (Expression *element: listLit->elements) {
                        elements.push_back(evaluate(element, currentDepth + 1));
                    }
                    valueStack.push_back(Value(elements));
                } else {
                    throw std::runtime_error("Unknown expression type");
                }
            } catch (ReturnException &e) {
                valueStack.push_back(e.value);
            }
        }

        return valueStack.back();
    }

    void printValue(const Value& value) {
        if (value.type == Value::STRING) {
            std::cout << value.strValue;
        } else if (value.type == Value::NUMBER) {
            std::cout << value.numValue;
        } else if (value.type == Value::LIST) {
            std::cout << "[";
            for (size_t i = 0; i < value.listValue.size(); ++i) {
                printValue(value.listValue[i]);
                if (i < value.listValue.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "]";
        } else if (value.type == Value::NULL_TYPE) {
            std::cout << "null";
        }
    }

    size_t utf8len(const std::string& str) {
        size_t charCount = 0;
        for (size_t i = 0; i < str.size();) {
            unsigned char c = static_cast<unsigned char>(str[i]);
            int charWidth = 1;

            if ((c & 0xF8) == 0xF0 && i + 3 < str.size()) {
                charWidth = 4;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < str.size()) {
                charWidth = 3;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
                charWidth = 2;
            }

            if (i + charWidth > str.size()) {
                charWidth = 1;
            }

            bool isValid = true;
            for (int j = 1; j < charWidth; j++) {
                if ((str[i + j] & 0xC0) != 0x80) {
                    isValid = false;
                    break;
                }
            }

            if (isValid) {
                i += charWidth;
                charCount++;
            } else {
                i += 1;
            }
        }
        return charCount;
    }
};

#endif