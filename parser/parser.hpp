#ifndef PARSER_HPP
#define PARSER_HPP

#include "../lexer/token.hpp"
#include "../ast/ast.hpp"
#include "errors.hpp"
#include <vector>

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::vector<Statement*> parse() {
        std::vector<Statement*> statements;
        while (peek().type != TOKEN_EOF) {
            statements.push_back(statement());
        }
        return statements;
    }

private:
    std::vector<Token> tokens;
    size_t position;

    Token peek(int offset = 0) {
        if (position + offset >= tokens.size())
            throwSyntaxError("Unexpected end of file");
        return tokens[position + offset];
    }

    void consume() {
        position++;
    }

    Statement* statement() {
        Token token = peek();
        if (token.type == TOKEN_IDENTIFIER && peek(1).type == TOKEN_PUNCTUATION && peek(1).value == "[") {
            size_t savedPos = position;
            try {
                return subscriptAssignment();
            } catch (const std::runtime_error& e) {
                position = savedPos;
                Expression* expr = expression();
                return new ExpressionStatement(expr);
            }
        } else if (token.type == TOKEN_IDENTIFIER && peek(1).type == TOKEN_OPERATOR && peek(1).value == "=") {
            return assignment();
        } else if (token.type == TOKEN_IDENTIFIER && peek(1).type == TOKEN_OPERATOR && peek(1).value == ".") {
            size_t savedPos = position;
            try {
                if (peek(3).type == TOKEN_PUNCTUATION && peek(3).value == "[") {
                    std::string className = token.value;
                    consume();
                    consume();
                    std::string memberName = peek().value;
                    consume();
                    consume();
                    Expression* index = expression();
                    if (peek().type != TOKEN_PUNCTUATION || peek().value != "]") {
                        throwSyntaxError("Expected ']' after list index");
                    }
                    consume();
                    if (peek().type != TOKEN_OPERATOR || peek().value != "=") {
                        throwSyntaxError("Expected '=' after list index");
                    }
                    consume();
                    Expression* value = expression();
                    return new ClassMemberAssignment(className, memberName, value, index);
                } else {
                    return classMemberAssignment();
                }
            } catch (const std::runtime_error& e) {
                position = savedPos;
                Expression* expr = expression();
                return new ExpressionStatement(expr);
            }
        } else if (token.type == TOKEN_KEYWORD && token.value == "if") {
            return ifStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "while") {
            return whileStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "fn") {
            return functionDeclaration();
        } else if (token.type == TOKEN_KEYWORD && token.value == "return") {
            return returnStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "break") {
            consume();
            return new BreakStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "continue") {
            consume();
            return new ContinueStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "for") {
            return forStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "class") {
            return classDeclaration();
        } else if (token.type == TOKEN_KEYWORD && token.value == "const") {
            return constantDeclaration();
        } else if (token.type == TOKEN_KEYWORD && token.value == "import") {
            return importStatement();
        } else if (token.type == TOKEN_KEYWORD && token.value == "raise") {
            return raiseStatement();
        }
        else {
            Expression* expr = expression();
            return new ExpressionStatement(expr);
        }
    }

    ImportStatement* importStatement() {
        consume();
        Token packageToken = peek();
        if (packageToken.type != TOKEN_STRING) {
            throwSyntaxError("Expected string literal after 'import'");
        }
        consume();
        return new ImportStatement(packageToken.value);
    }
    
    RaiseStatement* raiseStatement() {
        consume();
        Expression* errorMessage = expression();
        return new RaiseStatement(errorMessage);
    }

    ConstantDeclaration* constantDeclaration() {
        consume();
        Token name = peek();
        if (name.type != TOKEN_IDENTIFIER) {
            throwSyntaxError("Expected identifier after 'const'");
        }
        consume();
        
        if (peek().type != TOKEN_OPERATOR || peek().value != "=") {
            throwSyntaxError("Expected '=' after constant name");
        }
        consume();
        
        Expression* value = expression();
        return new ConstantDeclaration(name.value, value);
    }



    ClassDeclaration* classDeclaration() {
        consume();
        Token name = peek(), parentName = Token{TOKEN_EOF, ""};
        consume();
        if (peek().type == TOKEN_PUNCTUATION && peek().value == ":") {
            consume();
            parentName = peek();
            if (parentName.type != TOKEN_IDENTIFIER) throwSyntaxError("Expected class name after ':'");
            consume();
        }
        std::map<std::string, Assignment*> members;
        std::map<std::string, FunctionDeclaration*> functions;
        while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
            if (peek().type == TOKEN_KEYWORD && peek().value == "fn") {
                FunctionDeclaration* funcDecl = functionDeclaration();
                functions[funcDecl->name] = funcDecl;
            } else {
                Statement* stmt = statement();
                if (auto assign = dynamic_cast<Assignment*>(stmt)) {
                    members[assign->target] = assign;
                } else {
                    throwSyntaxError("Unsupported statement in class declaration");
                }
            }
        }
        consume();
        if (parentName.type != TOKEN_EOF) {
            return new ClassDeclaration(name.value, members, functions, parentName.value);
        }
        return new ClassDeclaration(name.value, members, functions);
    }

    Assignment* subscriptAssignment() {
        Token target = peek();
        consume();
        consume();
        Expression* index = expression();
        if (peek().type != TOKEN_PUNCTUATION || peek().value != "]") {
            throwSyntaxError("Expected ']' after list index");
        }
        consume();
        if (peek().type != TOKEN_OPERATOR || peek().value != "=") {
            throwSyntaxError("Expected '=' after list index");
        }
        consume();
        Expression* value = expression();
        return new Assignment(target.value, index, value);
    }

    ClassMemberAssignment* classMemberAssignment() {
        Token target = peek();
        consume();
        consume();
        std::string member = peek().value;
        consume();
        if (peek().type != TOKEN_OPERATOR || peek().value != "=") {
            throwSyntaxError("Expected '=' after member name");
        }
        consume();
        Expression* value = expression();
        return new ClassMemberAssignment(target.value, member, value);
    }

    Assignment* assignment() {
        Token target = peek();
        consume();
        consume();
        Expression* value = expression();
        return new Assignment(target.value, value);
    }

    IfStatement* ifStatement() {
        consume();
        Expression* condition = expression();

        std::vector<Statement*> body;
        while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
            if ((peek().type == TOKEN_KEYWORD && peek().value == "elif") || (peek().type == TOKEN_KEYWORD && peek().value == "else")) {
                break;
            }
            body.push_back(statement());
        }
        std::vector<Statement*> elifBody;
        std::vector<std::pair<Expression*, std::vector<Statement*>>> elifStatements;
        while (peek().type == TOKEN_KEYWORD && peek().value == "elif") {
            consume();
            Expression* elifCondition = expression();
            while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
                if ((peek().type == TOKEN_KEYWORD && peek().value == "else") || (peek().type == TOKEN_KEYWORD && peek().value == "elif")) {
                    break;
                }
                elifBody.push_back(statement());
            }
            elifStatements.push_back({elifCondition, elifBody});
            elifBody.clear();
        }

        std::vector<Statement*> elseBody;
        if (peek().type == TOKEN_KEYWORD && peek().value == "else") {
            consume();
            
            while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
                elseBody.push_back(statement());
            }
        }

        if (peek().type == TOKEN_KEYWORD && peek().value == "end") {
            consume();
        } else {
            throwSyntaxError("Expected 'end' to close if statement");
        }

        return new IfStatement(condition, body, elifStatements, elseBody);
    }

    ForStatement* forStatement() {
        consume();


        if (peek().type != TOKEN_IDENTIFIER) {
            throwSyntaxError("Expected identifier after 'for'");
        }
        std::string varName = peek().value;
        consume();


        if (peek().type != TOKEN_KEYWORD || peek().value != "in") {
            throwSyntaxError("Expected 'in' after for loop variable");
        }
        consume();


        Expression* iterable = expression();


        std::vector<Statement*> body;
        while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
            body.push_back(statement());
        }
        consume();

        return new ForStatement(varName, iterable, body);
    }

    WhileStatement* whileStatement() {
        consume();
        Expression* condition = expression();
        std::vector<Statement*> body;
        while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
            body.push_back(statement());
        }
        consume();
        return new WhileStatement(condition, body);
    }

    FunctionDeclaration* functionDeclaration() {
        consume();
        Token name = peek();
        consume();
        consume();
        std::vector<std::string> parameters;
        std::vector<Expression*> default_values;
        while (peek().type != TOKEN_PUNCTUATION || peek().value != ")") {
            parameters.push_back(peek().value);
            consume();
            if (peek().type == TOKEN_OPERATOR && peek().value == "=") {
                consume();
                default_values.push_back(expression());
            } else {
                default_values.push_back(nullptr);
            }
            if (peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                consume();
            }
        }
        consume();
        std::vector<Statement*> body;
        while (peek().type != TOKEN_KEYWORD || peek().value != "end") {
            body.push_back(statement());
        }
        consume();
        return new FunctionDeclaration(name.value, parameters, default_values, body);
    }

    ReturnStatement* returnStatement() {
        consume();
        Expression* value = expression();
        return new ReturnStatement(value);
    }

    Expression* expression() {
        return logical_or_expression();
    }

    Expression* logical_or_expression() {
        Expression* left = logical_and_expression();
        while (peek().type == TOKEN_KEYWORD && peek().value == "or") {
            Token op = peek();
            consume();
            Expression* right = logical_and_expression();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* logical_and_expression() {
        Expression* left = equality_expression();
        while (peek().type == TOKEN_KEYWORD && peek().value == "and") {
            Token op = peek();
            consume();
            Expression* right = equality_expression();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* equality_expression() {
        Expression* left = comparison_expression();
        while (peek().type == TOKEN_OPERATOR && (peek().value == "==" || peek().value == "!=")) {
            Token op = peek();
            consume();
            Expression* right = comparison_expression();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* comparison_expression() {
        Expression* left = arithmetic_expression();
        while (peek().type == TOKEN_OPERATOR &&
               (peek().value == "<" || peek().value == "<=" ||
                peek().value == ">" || peek().value == ">=" ||
                peek().value == "==" || peek().value == "!=")) {
            Token op = peek();
            consume();
            Expression* right = arithmetic_expression();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* arithmetic_expression() {
        Expression* left = term();
        while (peek().type == TOKEN_OPERATOR && (peek().value == "+" || peek().value == "-")) {
            Token op = peek();
            consume();
            Expression* right = term();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* term() {
        Expression* left = factor();
        while (peek().type == TOKEN_OPERATOR && (peek().value == "*" || peek().value == "/" || peek().value == "%" || peek().value == "^" || peek().value == "&" || peek().value == "|" || peek().value == "~")) {
            Token op = peek();
            consume();
            Expression* right = factor();
            left = new BinaryExpression(op.value, left, right);
        }
        return left;
    }

    Expression* factor() {
        Token token = peek();
        if (token.type == TOKEN_NUMBER) {
            consume();
            return new NumberLiteral(BigNum(token.value));
        } else if (token.type == TOKEN_OPERATOR && token.value == "-") {
            consume();
            Expression* expr = factor();
            return new UnaryExpression("-", expr);
        } else if (token.type == TOKEN_OPERATOR && token.value == "~") {
            return new NumberLiteral(BigNum(0));
        } else if (token.type == TOKEN_KEYWORD && token.value == "not") {
            consume();
            Expression* expr = primary();
            return new UnaryExpression("not", expr);
        } else {
            return primary();
        }
    }

    Expression* primary() {
        Token token = peek();
        if (token.type == TOKEN_NUMBER) {
            consume();
            return new NumberLiteral(BigNum(token.value));
        } else if (token.type == TOKEN_STRING) {
            consume();
            return new StringLiteral(token.value);
        } else if (token.type == TOKEN_KEYWORD && (token.value == "true" || token.value == "false")) {
            if (token.value == "true") {
                consume();
                return new NumberLiteral(BigNum(1));
            } else if (token.value == "false") {
                consume();
                return new NumberLiteral(BigNum(0));
            }
        } else if (token.type == TOKEN_KEYWORD && token.value == "null") {
            consume();
            return new NullLiteral();
        } else if (token.type == TOKEN_KEYWORD && token.value == "not") {
            consume();
            Expression* expr = primary();
            return new UnaryExpression("not", expr);
        } else if (token.type == TOKEN_IDENTIFIER) {
            consume();
            std::vector<Expression*> objects;
            objects.push_back(new Identifier(token.value));
            while (peek().value == ".") {
                consume();
                Token member = peek();
                if (member.type != TOKEN_IDENTIFIER) throwSyntaxError("Expected member name");
                consume();
                if (peek().type == TOKEN_PUNCTUATION && peek().value == "(") {
                    consume();
                    std::vector<Expression*> args;
                    while (true) {
                        if (peek().type == TOKEN_PUNCTUATION && peek().value == ")") {
                            break;
                        }
                        if (!args.empty() && peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                            consume();
                        }
                        args.push_back(expression());
                    }
                    consume();
                    std::string fullName = "";
                    if (auto id = dynamic_cast<Identifier*>(objects[0])) {
                        fullName = id->name;
                    }
                    for (size_t i = 1; i < objects.size(); i++) {
                        if (auto id = dynamic_cast<Identifier*>(objects[i])) {
                            fullName += "." + id->name;
                        }
                    }
                    fullName += "." + member.value;
                    return new FunctionCall(fullName, args);
                }
                objects.push_back(new Identifier(member.value));
                if (peek().type == TOKEN_PUNCTUATION && peek().value == "[") {
                    consume();
                    Expression* index = expression();
                    if (peek().type != TOKEN_PUNCTUATION || peek().value != "]") {
                        throwSyntaxError("Expected ']' after list index");
                    }
                    consume();
                    return new MemberAccess(objects, index);
                }
            }
            if (objects.size() > 1) {
                return new MemberAccess(objects);
            }
            if (peek().type == TOKEN_PUNCTUATION && peek().value == "(") {
                consume();
                std::vector<Expression*> args;
                while (true) {
                    if (peek().type == TOKEN_PUNCTUATION && peek().value == ")") {
                        break;
                    }
                    if (!args.empty() && peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                        consume();
                    }
                    args.push_back(expression());
                }
                consume();
                return new FunctionCall(token.value, args);
            } else if (peek().type == TOKEN_PUNCTUATION && peek().value == "[") {
                consume();
                Expression* index = expression();
                if (peek().type != TOKEN_PUNCTUATION || peek().value != "]") {
                    throwSyntaxError("Expected ']' after list index");
                }
                consume();
                return new BinaryExpression("[]", new Identifier(token.value), index);
            } else {
                return new Identifier(token.value);
            }
        } else if (token.type == TOKEN_PUNCTUATION && token.value == "(") {
            consume();
            Expression* expr = expression();
            consume();
            return expr;
        } else if (token.type == TOKEN_PUNCTUATION && token.value == "[") {
            consume();
            std::vector<Expression*> elements;
            if (peek().type != TOKEN_PUNCTUATION || peek().value != "]") {
                while (true) {
                    elements.push_back(expression());
                    if (peek().type == TOKEN_PUNCTUATION && peek().value == "]") {
                        break;
                    } else if (peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                        consume();
                    } else {
                        throwSyntaxError("Expected ',' or ']' in list literal");
                    }
                }
            }
            consume();
            return new ListLiteral(elements);
        } else if (token.type == TOKEN_KEYWORD && token.value == "new") {
            consume();
            Token className = peek();
            if (className.type != TOKEN_IDENTIFIER) throwSyntaxError("Expected class name after new");
            consume();
            if (peek().type == TOKEN_PUNCTUATION && peek().value == "(") {
                consume();
                std::vector<Expression*> args;
                while (true) {
                    if (peek().type == TOKEN_PUNCTUATION && peek().value == ")") {
                        break;
                    }
                    if (!args.empty() && peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                        consume();
                    }
                    args.push_back(expression());
                }
                consume();
                return new NewExpression(className.value, args, true);
            }
            return new NewExpression(className.value);
        } else {
            throwSyntaxError("Unexpected token in primary expression: " + token.value);
        }
    }
};

#endif