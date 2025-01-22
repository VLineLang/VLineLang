cmake_minimum_required(VERSION 3.30)
project(Vline_lang)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(
        Vline_lang main.cpp
        core/parser.hpp
        core/ast.hpp
        core/core.hpp
        core/token.hpp
        core/value.hpp
        core/interpreter.hpp
        core/lexer.hpp
        core/errors.hpp
        core/bytecode.hpp
        core/codegen.hpp
        core/vm.hpp
)
