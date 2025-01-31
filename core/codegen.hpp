#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "ast.hpp"
#include "bytecode.hpp"
#include <map>
#include <vector>
#include <cmath>

class CodeGen {
public:
    BytecodeProgram generate(const std::vector<Statement*>& statements) {
        BytecodeProgram program;
        for (Statement* stmt : statements) {
            generateStatement(stmt, program);
        }
        return program;
    }

    void setReplMode(bool mode) {
        isReplMode = mode;
    }

    std::map<std::string, FunctionDeclaration*> getFunctions() const {
        return functions;
    }

private:
    bool isReplMode = false;
    bool inFunction = false;
    std::map<std::string, FunctionDeclaration*> functions;
    std::map<std::string, int> variables;
    std::map<std::string, int> labels;
    int labelCounter = 0;

    int tempVarCounter = 0;

    void generateStatement(Statement* stmt, BytecodeProgram& program) {
        if (auto assignment = dynamic_cast<Assignment*>(stmt)) {
            if (assignment->isSubscriptAssignment) {
                generateExpression(new Identifier(assignment->target), program);
                generateExpression(assignment->index, program);
                generateExpression(assignment->value, program);
                program.push_back({STORE_SUBSCRIPT});
                program.push_back({STORE_VAR, assignment->target});
            } else {
                generateExpression(assignment->value, program);
                program.push_back({STORE_VAR, assignment->target});
            }
        }
        else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            generateExpression(ifStmt->condition, program);
            program.push_back({JUMP_IF_FALSE, createLabel()});
            size_t falseJumpPos = program.size() - 1;

            for (Statement* bodyStmt : ifStmt->body) {
                generateStatement(bodyStmt, program);
            }

            if (!ifStmt->elseBody.empty()) {
                program.push_back({JUMP, createLabel()});
                size_t endJumpPos = program.size() - 1;

                program[falseJumpPos].operand = (int)program.size();
                for (Statement* elseStmt : ifStmt->elseBody) {
                    generateStatement(elseStmt, program);
                }
                program[endJumpPos].operand = (int)program.size();
            } else {
                program[falseJumpPos].operand = (int)program.size();
            }
        }
        else if (auto forStmt = dynamic_cast<ForStatement*>(stmt)) {
            generateExpression(forStmt->iterable, program);
            std::string tempVar = "__iter_temp_" + std::to_string(tempVarCounter++);
            program.push_back({STORE_VAR, tempVar});
            program.push_back({LOAD_VAR, tempVar});
            program.push_back({GET_ITER});
            size_t loopStart = program.size();

            program.push_back({FOR_ITER, createLabel()});
            size_t exitPos = program.size() - 1;

            program.push_back({STORE_VAR, forStmt->variable});


            for (Statement* bodyStmt : forStmt->body) {
                generateStatement(bodyStmt, program);
                program.push_back({POP, 0});
            }

            program.push_back({JUMP, static_cast<int>(loopStart)});
            program[exitPos].operand = static_cast<int>(program.size());
        }
        else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            size_t loopStart = program.size();
            generateExpression(whileStmt->condition, program);
            program.push_back({JUMP_IF_FALSE, createLabel()});
            size_t exitPos = program.size() - 1;

            for (Statement* bodyStmt : whileStmt->body) {
                generateStatement(bodyStmt, program);
            }

            program.push_back({JUMP, static_cast<int>(loopStart)});
            program[exitPos].operand = static_cast<int>(program.size());
        }
        else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt)) {
            functions[funcDecl->name] = funcDecl;
            BytecodeProgram funcProgram;


            inFunction = true;
            for (Statement* bodyStmt : funcDecl->body) {
                generateStatement(bodyStmt, funcProgram);
            }
            inFunction = false;


            if (funcProgram.empty() || funcProgram.back().op != RETURN) {
                funcProgram.push_back({LOAD_CONST, 0.0});
                funcProgram.push_back({RETURN, 0});
            }
            funcDecl->bytecode = funcProgram;
        }
        else if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {

            if (!inFunction) {
                throwSyntaxError("'return' outside function");
            }
            generateExpression(returnStmt->value, program);
            program.push_back({RETURN, 0});
        }
        else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            generateExpression(exprStmt->expression, program);
            if (!isReplMode) {
                program.push_back({POP, 0});
            }
        }
    }

    void generateExpression(Expression* expr, BytecodeProgram& program) {
        if (auto numLit = dynamic_cast<NumberLiteral*>(expr)) {
            program.push_back({LOAD_CONST, numLit->value});
        }
        else if (auto strLit = dynamic_cast<StringLiteral*>(expr)) {
            program.push_back({LOAD_CONST, strLit->value});
        }
        else if (auto listLit = dynamic_cast<ListLiteral*>(expr)) {
            for (auto element : listLit->elements) {
                generateExpression(element, program);
            }
            program.push_back({BUILD_LIST, static_cast<int>(listLit->elements.size())});
        }
        else if (auto id = dynamic_cast<Identifier*>(expr)) {
            program.push_back({LOAD_VAR, id->name});
        }
        else if (auto binExpr = dynamic_cast<BinaryExpression*>(expr)) {
            if (binExpr->op == "[]") { // 处理下标访问
                generateExpression(binExpr->left, program);  // 加载列表
                generateExpression(binExpr->right, program); // 加载索引
                program.push_back({LOAD_SUBSCRIPT});         // 生成加载下标指令
            } else {
                handleBinaryOp(binExpr, program);
            }
        }
        else if (auto unaryExpr = dynamic_cast<UnaryExpression*>(expr)) {
            if (unaryExpr->op == "-") {
                program.push_back({LOAD_CONST, 0.0});
                generateExpression(unaryExpr->expr, program);
                program.push_back({BINARY_OP, "-"});
            } else if (unaryExpr->op == "not") {
                generateExpression(unaryExpr->expr, program);
                program.push_back({LOAD_CONST, 0.0});
                program.push_back({BINARY_OP, "=="});
            }
        }
        else if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
            for (auto arg : funcCall->arguments) {
                generateExpression(arg, program);
            }
            program.push_back({CALL_FUNCTION, CallFunctionOperand{funcCall->name, static_cast<int>(funcCall->arguments.size())}});
        }
    }

    void handleBinaryOp(BinaryExpression* expr, BytecodeProgram& program) {
        generateExpression(expr->left, program);
        generateExpression(expr->right, program);

        program.push_back({BINARY_OP, expr->op});
    }

    int createLabel() { return labelCounter++; }
};

#endif