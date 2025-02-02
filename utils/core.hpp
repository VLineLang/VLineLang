#ifndef CORE_HPP
#define CORE_HPP

#define VLINE_VERSION "0.5.4-alpha.1"
#define VLINE_PUBLISH "Feb. 1st, 2025"
#define VLINE_COMPILER "GNU GCC/ISO C++17"

#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../parser/value.hpp"
#include "../parser/parser.hpp"
#include "../parser/errors.hpp"
#include "../vm/vm.hpp"
#include "../bytecode/bytecode.hpp"
#include "../bytecode/codegen.hpp"
#include "../vm/bignum.hpp"

#endif