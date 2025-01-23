#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include "errors.hpp"
#include <string>
#include <vector>
#include <cctype>

class Lexer {
public:
    Lexer(const std::string& source) : source(source), position(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (position < source.length()) {
            char current = peek();
            if (std::isspace(current)) {
                consume();
            } else if (current == '/' && peek(1) == '/') {
                skipSingleLineComment();
            } else if (current == '/' && peek(1) == '*') {
                skipMultiLineComment();
            } else if (std::isalpha(current)) {
                tokens.push_back(identifier());
            } else if (std::isdigit(current)) {
                tokens.push_back(number());
            } else if (current == '"') {
                tokens.push_back(string());
            } else if (current == '.' || current == '=' || current == '+' ||
                       current == '*' || current == '/' || current == '<' ||
                       current == '>' || current == '!' || current == '-') {
                tokens.push_back(operator_());
            } else if (current == '(' || current == ')' || current == '{' ||
                       current == '}' || current == ',' || current == '[' || current == ']') {
                tokens.push_back(punctuation());
            } else {
                throwSyntaxError("Unexpected character: " + std::string(1, current));
            }
        }
        tokens.push_back({TOKEN_EOF, ""});
        return tokens;
    }

private:
    std::string source;
    size_t position;
    std::vector<Token> tokens;

    char peek(int offset = 0) {
        if (position + offset >= source.length()) {
            return '\0';
        }
        return source[position + offset];
    }

    void consume() {
        position++;
    }

    void skipSingleLineComment() {
        while (position < source.length() && peek() != '\n') {
            consume();
        }
        consume();
    }

    void skipMultiLineComment() {
        consume();
        consume();
        while (position < source.length()) {
            if (peek() == '*' && peek(1) == '/') {
                consume();
                consume();
                break;
            }
            consume();
        }
    }

    Token identifier() {
        std::string value;
        while (std::isalnum(peek()) || peek() == '_') {
            value += peek();
            consume();
        }
        if (value == "fn" || value == "if" || value == "while" ||
            value == "return" || value == "true" || value == "false" ||
            value == "and" || value == "or" || value == "not" || value == "else" ||
            value == "break" || value == "continue" || value == "null" ||
            value == "for" || value == "in") {
            return {TOKEN_KEYWORD, value};
        }
        return {TOKEN_IDENTIFIER, value};
    }

    Token number() {
        std::string value;
        bool hasDot = false;
        while (std::isdigit(peek()) || (peek() == '.' && !hasDot)) {
            if (peek() == '.') {
                hasDot = true;
            }
            value += peek();
            consume();
        }
        return {TOKEN_NUMBER, value};
    }

    Token string() {
        consume();
        std::string value;
        while (peek() != '"') {
            if (peek() == '\\') {
                consume();
                char next = peek();
                switch (next) {
                    case 'n':
                        value += '\n';
                        break;
                    case 't':
                        value += '\t';
                        break;
                    case '"':
                        value += '"';
                        break;
                    case '\\':
                        value += '\\';
                        break;
                    default:
                        value += '\\' + next;
                        break;
                }
                consume();
            } else {
                value += peek();
                consume();
            }
        }
        consume();
        return {TOKEN_STRING, value};
    }

    Token operator_() {
        std::string op;
        char current = peek();
        if (current == '.' || current == '=' || current == '+' ||
            current == '*' || current == '/' || current == '<' ||
            current == '>' || current == '!') {
            op += current;
            consume();
            if (op == "=" || op == "!" || op == "<" || op == ">") {
                if (peek() == '=') {
                    op += '=';
                    consume();
                }
            }
            return {TOKEN_OPERATOR, op};
        } else if (current == '-') {
            op += current;
            consume();
            return {TOKEN_OPERATOR, op};
        } else {
            throwSyntaxError("Unexpected character: " + std::string(1, current));
        }
    }

    Token punctuation() {
        char punc = peek();
        if (punc == '(' || punc == ')' || punc == '{' ||
            punc == '}' || punc == ',' || punc == '[' || punc == ']') {
            consume();
            return {TOKEN_PUNCTUATION, std::string(1, punc)};
        } else {
            throwSyntaxError("Unexpected character: " + std::string(1, punc));
        }
    }
};

#endif