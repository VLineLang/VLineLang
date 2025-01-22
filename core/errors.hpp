#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>
#include <string>

const std::string SYNTAX_ERROR = "Syntax Error: ";
const std::string IDENTIFIER_ERROR = "Identifier Error: ";
const std::string TYPE_ERROR = "Type Error: ";
const std::string INDEX_ERROR = "Index Error: ";
const std::string IO_ERROR = "IO Error: ";
const std::string ZERO_DIVISION_ERROR = "Zero Division Error: ";
const std::string RECURSION_ERROR = "Recursion Error: ";

inline void throwSyntaxError(const std::string& msg) {
    throw std::runtime_error(SYNTAX_ERROR + msg);
}

inline void throwIdentifierError(const std::string& msg) {
    throw std::runtime_error(IDENTIFIER_ERROR + msg);
}

inline void throwTypeError(const std::string& msg) {
    throw std::runtime_error(TYPE_ERROR + msg);
}

inline void throwIndexError(const std::string& msg) {
    throw std::runtime_error(INDEX_ERROR + msg);
}

inline void throwIOError(const std::string& msg) {
    throw std::runtime_error(IO_ERROR + msg);
}

inline void throwZeroDivisionError(const std::string& msg) {
    throw std::runtime_error(ZERO_DIVISION_ERROR + msg);
}

inline void throwRecursionError(const std::string& msg) {
    throw std::runtime_error(RECURSION_ERROR + msg);
}

#endif