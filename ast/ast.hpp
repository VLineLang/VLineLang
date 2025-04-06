#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include "../bytecode/bytecode.hpp"

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expression : ASTNode {
    virtual std::string toString() const = 0;
};

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
        IMPORT_STATEMENT,
        RAISE_STATEMENT
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
    std::vector<Expression*> default_values;
    std::vector<Statement*> body;
    BytecodeProgram bytecode;

    FunctionDeclaration(const std::string& name, const std::vector<std::string>& parameters, const std::vector<Expression*>& default_values, const std::vector<Statement*>& body)
            : name(name), parameters(parameters), default_values(default_values), body(body) {}

    StatementType type() const override {
        return FUNCTION_DECLARATION;
    }
};

struct FunctionCall : Expression {
    std::string name;
    std::vector<Expression*> arguments;

    FunctionCall(const std::string& name, const std::vector<Expression*>& arguments)
            : name(name), arguments(arguments) {}
    std::string toString() const override {
        std::string result = name + "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += arguments[i]->toString();
        }
        result += ")";
        return result;
    }
};

struct BinaryExpression : Expression {
    std::string op;
    Expression* left;
    Expression* right;

    BinaryExpression(const std::string& op, Expression* left, Expression* right)
            : op(op), left(left), right(right) {}
    std::string toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }
};

struct UnaryExpression : Expression {
    std::string op;
    Expression* expr;

    UnaryExpression(const std::string& op, Expression* expr)
            : op(op), expr(expr) {}
    std::string toString() const override {
        return op + expr->toString();
    }
};

struct NumberLiteral : Expression {
    BigNum value;
    NumberLiteral(const BigNum& val) : value(val) {}
    std::string toString() const override {
        return value.to_string();
    }
};

struct StringLiteral : Expression {
    std::string value;

    StringLiteral(const std::string& value) : value(value) {}
    std::string toString() const override {
        return value;
    }
};

struct NullLiteral : Expression {
    NullLiteral() = default;
    std::string toString() const override {
        return "null";
    }
};

struct Identifier : Expression {
    std::string name;

    Identifier(const std::string& name) : name(name) {}
    std::string toString() const override {
        return name;
    }
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

struct RaiseStatement : Statement {
    Expression* errorMessage;
    
    RaiseStatement(Expression* errorMessage) : errorMessage(errorMessage) {}
    
    StatementType type() const override {
        return RAISE_STATEMENT;
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
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) result += ", ";
            result += elements[i]->toString();
        }
        result += "]";
        return result;
    }
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
    Expression* index;
    bool hasIndex;

    ClassMemberAssignment(const std::string& className, const std::string& memberName, Expression* value, Expression* idx = nullptr)
            : className(className), memberName(memberName), value(value), index(idx), hasIndex(idx != nullptr) {}

    StatementType type() const override { return CLASS_MEMBER_ASSIGNMENT; }
};

struct NewExpression : public Expression {
    std::string className;
    std::vector<Expression*> args_init;
    bool is_init;
    NewExpression(const std::string& name,
                  const std::vector<Expression*>& args = {},
                  bool init = false) : className(name), args_init(args), is_init(init) {}
    std::string toString() const override {
        std::string result = "new " + className;
        if (!args_init.empty()) {
            result += "(";
            for (size_t i = 0; i < args_init.size(); ++i) {
                if (i > 0) result += ", ";
                result += args_init[i]->toString();
            }
            result += ")";
        }
        return result;
    }
};

struct MemberAccess : public Expression {
    std::vector<Expression*> objects;
    Expression* index;
    bool hasIndex;

    MemberAccess(const std::vector<Expression*>& objs, Expression* idx = nullptr) 
        : objects(objs), index(idx), hasIndex(idx != nullptr) {}

    std::string toString() const {
        std::string result = objects[0]->toString();
        for (size_t i = 1; i < objects.size(); i++) {
            result += "." + objects[i]->toString();
        }
        if (hasIndex) {
            result += "[" + index->toString() + "]";
        }
        return result;
    }
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