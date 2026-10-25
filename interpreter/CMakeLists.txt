cmake_minimum_required(VERSION 3.15)

project(F6_Interpreter
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(f6_interpreter
    src/main.c
)