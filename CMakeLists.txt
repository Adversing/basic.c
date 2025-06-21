cmake_minimum_required(VERSION 3.29)
project(basic C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(basic
        src/interpreter/basic_interpreter.c
        src/interpreter/basic_interpreter.h
        src/tokenizer/tokenizer.c
        src/eval/expression_evaluator.c
        src/cmd/command_executor.c
        src/main.c)
