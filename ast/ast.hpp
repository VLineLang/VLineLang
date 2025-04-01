#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include "../bytecode/bytecode.hpp"

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expression : ASTNode {};

struct Statement : ASTNode {
    enum StatementType {
        ASSIGNMENT,
        IF_STATEMENT,
        WHILE_STATEMENT,
        FUNCTION_DECLARATION,
        RETURN_STATEMENT,
        BREAK_STATEMENT,
        CONTINUE_STATEMENT,
        EXPRESSION_STATEMENT,
        FOR_STATEMENT,
        CLASS_DECLARATION,
        CLASS_MEMBER_ASSIGNMENT,
        CONSTANT_DECLARATION,
        IMPORT_STATEMENT
    };

    virtual StatementType type() const = 0;
};

struct ImportStatement : Statement {
    std::string packageName;

    ImportStatement(const std::string& packageName) : packageName(packageName) {}

    StatementType type() const override {
        return IMPORT_STATEMENT;
    }
};

struct Assignment : Statement {
    std::string target;
    Expression* index;
    Expression* value;
    bool isSubscriptAssignment;

    Assignment(const std::string& target, Expression* value)
            : target(target), index(nullptr), value(value), isSubscriptAssignment(false) {}

    Assignment(const std::string& target, Expression* index, Expression* value)
            : target(target), index(index), value(value), isSubscriptAssignment(true) {}

    StatementType type() const override {
        return ASSIGNMENT;
    }
};

struct IfStatement : Statement {
    Expression* condition;
    std::vector<Statement*> body;
    std::vector<Statement*> elseBody;
    std::vector<std::pair<Expression*, std::vector<Statement*>>> elifStatements;

    IfStatement(Expression* condition, const std::vector<Statement*>& body, const std::vector<std::pair<Expression*, std::vector<Statement*>>>& elifStatements = {}, const std::vector<Statement*>& elseBody = {})
            : condition(condition), body(body), elifStatements(elifStatements), elseBody(elseBody) {}

    StatementType type() const override {
        return IF_STATEMENT;
    }
};

struct WhileStatement : Statement {
    Expression* condition;
    std::vector<Statement*> body;

    WhileStatement(Expression* condition, const std::vector<Statement*>& body)
            : condition(condition), body(body) {}

    StatementType type() const override {
        return WHILE_STATEMENT;
    }
};


struct FunctionDeclaration : public Statement {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<Statement*> body;
    BytecodeProgram bytecode;

    FunctionDeclaration(const std::string& name, const std::vector<std::string>& parameters, const std::vector<Statement*>& body)
            : name(name), parameters(parameters), body(body) {}

    StatementType type() const override {
        return FUNCTION_DECLARATION;
    }
};

struct FunctionCall : Expression {
    std::string name;
    std::vector<Expression*> arguments;

    FunctionCall(const std::string& name, const std::vector<Expression*>& arguments)
            : name(name), arguments(arguments) {}
};

struct BinaryExpression : Expression {
    std::string op;
    Expression* left;
    Expression* right;

    BinaryExpression(const std::string& op, Expression* left, Expression* right)
            : op(op), left(left), right(right) {}
};

struct UnaryExpression : Expression {
    std::string op;
    Expression* expr;

    UnaryExpression(const std::string& op, Expression* expr)
            : op(op), expr(expr) {}
};

struct NumberLiteral : Expression {
    BigNum value;
    NumberLiteral(const BigNum& val) : value(val) {}
};

struct StringLiteral : Expression {
    std::string value;

    StringLiteral(const std::string& value) : value(value) {}
};

struct NullLiteral : Expression {
    NullLiteral() = default;
};

struct Identifier : Expression {
    std::string name;

    Identifier(const std::string& name) : name(name) {}
};

struct ReturnStatement : Statement {
    Expression* value;

    ReturnStatement(Expression* value) : value(value) {}

    StatementType type() const override {
        return RETURN_STATEMENT;
    }
};

struct BreakStatement : Statement {
    StatementType type() const override {
        return BREAK_STATEMENT;
    }
};

struct ContinueStatement : Statement {
    StatementType type() const override {
        return CONTINUE_STATEMENT;
    }
};

struct ExpressionStatement : Statement {
    Expression* expression;

    ExpressionStatement(Expression* expr) : expression(expr) {}

    StatementType type() const override {
        return EXPRESSION_STATEMENT;
    }
};

struct ListLiteral : Expression {
    std::vector<Expression*> elements;

    ListLiteral(const std::vector<Expression*>& elements) : elements(elements) {}
};

struct ForStatement : Statement {
    std::string variable;
    Expression* iterable;
    std::vector<Statement*> body;

    ForStatement(const std::string& var, Expression* iter, const std::vector<Statement*>& b)
            : variable(var), iterable(iter), body(b) {}

    StatementType type() const override {
        return FOR_STATEMENT;
    }
};

class ClassDeclaration : public Statement {
public:
    std::string className;
    std::map<std::string, Assignment*> members;
    std::map<std::string, FunctionDeclaration*> functions;
    std::string parentName;

    ClassDeclaration(const std::string& name,
                     const std::map<std::string, Assignment*>& m,
                     const std::map<std::string, FunctionDeclaration*>& f,
                     const std::string& pn = "self")
            : className(name), members(m), functions(f), parentName(pn) {}

    StatementType type() const override { return CLASS_DECLARATION; }
};

class ClassMemberAssignment : public Statement {
public:
    std::string className;
    std::string memberName;
    Expression* value;

    ClassMemberAssignment(const std::string& className, const std::string& memberName, Expression* value)
            : className(className), memberName(memberName), value(value) {}

    StatementType type() const override { return CLASS_MEMBER_ASSIGNMENT; }
};

struct NewExpression : public Expression {
    std::string className;
    std::vector<Expression*> args_init;
    bool is_init;
    NewExpression(const std::string& name,
                  const std::vector<Expression*>& args = {},
                  bool init = false) : className(name), args_init(args), is_init(init) {}
};

struct MemberAccess : public Expression {
    Expression* object;
    std::string member;

    MemberAccess(Expression* obj, const std::string& mem) : object(obj), member(mem) {}
};

struct ConstantDeclaration : Statement {
    std::string name;
    Expression* value;

    ConstantDeclaration(const std::string& name, Expression* value)
            : name(name), value(value) {}

    StatementType type() const override {
        return CONSTANT_DECLARATION;
    }
};

#endif