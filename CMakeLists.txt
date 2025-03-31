cmake_minimum_required(VERSION 3.30)
project(Vline_lang)

set(CMAKE_CXX_STANDARD 17)

# static compiler
set(CMAKE_CXX_FLAGS "-static")

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
        std/sys/sys.hpp
        std/std.hpp
        std/sys/time.hpp
        std/general/lists.hpp
        std/general/type.hpp
        std/sys/files.hpp
        std/general/IO.hpp
        utils/utils.hpp
        std/maths/math.hpp
)
