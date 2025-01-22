// codegen.hpp
#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "ast.hpp"
#include "bytecode.hpp"
#include <map>
#include <vector>

class CodeGen {
public:
    BytecodeProgram generate(const std::vector<Statement*>& statements) {
        BytecodeProgram program;
        for (Statement* stmt : statements) {
            generateStatement(stmt, program);
        }
        return program;
    }

private:
    std::map<std::string, int> variables;
    std::map<std::string, int> labels;
    int labelCounter = 0;

    void generateStatement(Statement* stmt, BytecodeProgram& program) {
        if (auto varDecl = dynamic_cast<VariableDeclaration*>(stmt)) {
            generateExpression(varDecl->initializer, program);
            program.push_back({STORE_VAR, varDecl->name});
        }
        else if (auto assignment = dynamic_cast<Assignment*>(stmt)) {
            generateExpression(assignment->value, program);
            program.push_back({STORE_VAR, assignment->target});
        }
        else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            generateExpression(ifStmt->condition, program);
            program.push_back({POP_JUMP_IF_FALSE, createLabel()});
            size_t falseJumpPos = program.size() - 1;

            for (Statement* bodyStmt : ifStmt->body) {
                generateStatement(bodyStmt, program);
            }

            if (!ifStmt->elseBody.empty()) {
                program.push_back({JUMP, createLabel()});
                size_t endJumpPos = program.size() - 1;

                program[falseJumpPos].operand = static_cast<int>(program.size());
                for (Statement* elseStmt : ifStmt->elseBody) {
                    generateStatement(elseStmt, program);
                }
                program[endJumpPos].operand = static_cast<int>(program.size());
            } else {
                program[falseJumpPos].operand = static_cast<int>(program.size());
            }
        }
        else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            size_t loopStart = program.size();
            generateExpression(whileStmt->condition, program);
            program.push_back({POP_JUMP_IF_FALSE, createLabel()});
            size_t exitPos = program.size() - 1;

            for (Statement* bodyStmt : whileStmt->body) {
                generateStatement(bodyStmt, program);
            }

            program.push_back({JUMP_ABSOLUTE, static_cast<int>(loopStart)});
            program[exitPos].operand = static_cast<int>(program.size());
        }
        else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt)) {

        }
        else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
            generateExpression(exprStmt->expression, program);
            program.push_back({POP, 0});
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
            handleBinaryOp(binExpr, program);
        }
        else if (auto unaryExpr = dynamic_cast<UnaryExpression*>(expr)) {
            generateExpression(unaryExpr->expr, program);
            program.push_back({UNARY_OP, unaryExpr->op});
        }
        else if (auto funcCall = dynamic_cast<FunctionCall*>(expr)) {
            for (auto arg : funcCall->arguments) {
                generateExpression(arg, program);
            }
            program.push_back({CALL_FUNCTION, CallFunctionOperand{funcCall->name,
                                                                  static_cast<int>(funcCall->arguments.size())}});
        }
    }

    void handleBinaryOp(BinaryExpression* expr, BytecodeProgram& program) {
        std::string op = expr->op;
        if (op == "and" || op == "or") {

        }
        else if (op == "<" || op == "<=" || op == "==" || op == "!=" || op == ">" || op == ">=") {
            generateExpression(expr->left, program);
            generateExpression(expr->right, program);
            program.push_back({COMPARE_OP, getCompareOp(op)});
        }
        else {
            generateExpression(expr->left, program);
            generateExpression(expr->right, program);
            program.push_back({BINARY_OP, op});
        }
    }

    CompareOp getCompareOp(const std::string& op) {
        if (op == "<") return CMP_LT;
        if (op == "<=") return CMP_LE;
        if (op == "==") return CMP_EQ;
        if (op == "!=") return CMP_NE;
        if (op == ">") return CMP_GT;
        if (op == ">=") return CMP_GE;
        throw std::runtime_error("Unknown comparison operator: " + op);
    }

    int createLabel() { return labelCounter++; }
};

#endif