#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string value;
};

#endif