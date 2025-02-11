#ifndef CORE_HPP
#define CORE_HPP

#define VLINE_VERSION "0.9.1-alpha.1"
#define VLINE_PUBLISH "Feb. 11th, 2025"
#define VLINE_COMPILER "GNU GCC/ISO C++17"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../utils/utils.hpp"
#include "../parser/errors.hpp"
#include "../vm/bignum.hpp"
#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../parser/parser.hpp"
#include "../bytecode/bytecode.hpp"
#include "../bytecode/codegen.hpp"
#include "../parser/value.hpp"
#include "../std/std.hpp"
#include "../vm/vm.hpp"

#endif