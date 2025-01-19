#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expression : ASTNode {};

struct Statement : ASTNode {
    enum StatementType {
        BLOCK,
        VARIABLE_DECLARATION,
        ASSIGNMENT,
        IF_STATEMENT,
        WHILE_STATEMENT,
        FUNCTION_DECLARATION,
        RETURN_STATEMENT,
        BREAK_STATEMENT,
        CONTINUE_STATEMENT,
        EXPRESSION_STATEMENT
    };

    virtual StatementType type() const = 0;
};

struct VariableDeclaration : Statement {
    std::string name;
    Expression* initializer;

    VariableDeclaration(const std::string& name, Expression* initializer)
            : name(name), initializer(initializer) {}

    StatementType type() const override {
        return VARIABLE_DECLARATION;
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

    IfStatement(Expression* condition, const std::vector<Statement*>& body, const std::vector<Statement*>& elseBody = {})
            : condition(condition), body(body), elseBody(elseBody) {}

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

struct FunctionDeclaration : Statement {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<Statement*> body;

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

struct NumberLiteral : Expression {
    double value;

    NumberLiteral(double value) : value(value) {}
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

struct BlockStatement : Statement {
    std::vector<Statement*> statements;

    BlockStatement(const std::vector<Statement*>& statements) : statements(statements) {}

    StatementType type() const override {
        return BLOCK;
    }
};

#endif