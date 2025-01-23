#ifndef PARSER_HPP
#define PARSER_HPP

#include "token.hpp"
#include "ast.hpp"
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
        } else {
            Expression* expr = expression();
            return new ExpressionStatement(expr);
        }
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

        if (peek().type != TOKEN_PUNCTUATION || peek().value != "{") {
            throwSyntaxError("Expected '{' after if condition");
        }
        consume();

        std::vector<Statement*> body;
        while (peek().type != TOKEN_PUNCTUATION || peek().value != "}") {
            body.push_back(statement());
        }
        consume();

        std::vector<Statement*> elseBody;
        if (peek().type == TOKEN_KEYWORD && peek().value == "else") {
            consume();
            if (peek().type == TOKEN_PUNCTUATION && peek().value == "{") {
                consume();
                while (peek().type != TOKEN_PUNCTUATION || peek().value != "}") {
                    elseBody.push_back(statement());
                }
                consume();
            } else {
                throwSyntaxError("Expected '{' after else");
            }
        }

        return new IfStatement(condition, body, elseBody);
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


        if (peek().type != TOKEN_PUNCTUATION || peek().value != "{") {
            throwSyntaxError("Expected '{' to start for loop body");
        }
        consume();

        std::vector<Statement*> body;
        while (peek().type != TOKEN_PUNCTUATION || peek().value != "}") {
            body.push_back(statement());
        }
        consume();

        return new ForStatement(varName, iterable, body);
    }

    WhileStatement* whileStatement() {
        consume();
        Expression* condition = expression();
        if (peek().type != TOKEN_PUNCTUATION || peek().value != "{") {
            throwSyntaxError("Expected '{' after while condition");
        }
        consume();
        std::vector<Statement*> body;
        while (peek().type != TOKEN_PUNCTUATION || peek().value != "}") {
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
        while (peek().type != TOKEN_PUNCTUATION || peek().value != ")") {
            parameters.push_back(peek().value);
            consume();
            if (peek().type == TOKEN_PUNCTUATION && peek().value == ",") {
                consume();
            }
        }
        consume();
        consume();
        std::vector<Statement*> body;
        while (peek().type != TOKEN_PUNCTUATION || peek().value != "}") {
            body.push_back(statement());
        }
        consume();
        return new FunctionDeclaration(name.value, parameters, body);
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
        while (peek().type == TOKEN_OPERATOR && (peek().value == "*" || peek().value == "/")) {
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
            return new NumberLiteral(std::stod(token.value));
        } else if (token.type == TOKEN_OPERATOR && token.value == "-") {
            consume();
            Expression* expr = factor();
            return new UnaryExpression("-", expr);
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
            return new NumberLiteral(std::stod(token.value));
        } else if (token.type == TOKEN_STRING) {
            consume();
            return new StringLiteral(token.value);
        } else if (token.type == TOKEN_KEYWORD && (token.value == "true" || token.value == "false")) {
            if (token.value == "true") {
                consume();
                return new NumberLiteral(1.0);
            } else if (token.value == "false") {
                consume();
                return new NumberLiteral(0.0);
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
            } else if (peek().type == TOKEN_OPERATOR && peek().value == ".") {
                consume();
                Token member = peek();
                if (member.type != TOKEN_IDENTIFIER) {
                    throwSyntaxError("Expected member function name after '.'");
                }
                consume();
                if (peek().type != TOKEN_PUNCTUATION || peek().value != "(") {
                    throwSyntaxError("Expected '(' after member function name");
                }
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
                return new FunctionCall(token.value + "." + member.value, args);
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
        } else {
            throwSyntaxError("Unexpected token in primary expression: " + token.value);
        }
    }
};

#endif