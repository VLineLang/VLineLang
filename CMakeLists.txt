cmake_minimum_required(VERSION 3.30)
project(Vline_lang)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(
        Vline_lang main.cpp
        parser/parser.hpp
        ast/ast.hpp
        utils/core.hpp
        lexer/token.hpp
        parser/value.hpp
        lexer/lexer.hpp
        parser/errors.hpp
        bytecode/bytecode.hpp
        bytecode/codegen.hpp
        vm/vm.hpp
        vm/bignum.hpp
)
