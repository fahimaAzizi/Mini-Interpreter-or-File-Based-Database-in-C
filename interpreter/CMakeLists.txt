
cmake_minimum_required(VERSION 3.15)

project(F6_Database
    VERSION 0.3.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Database library
add_library(f6_database_lib
    src/database.c
)

target_include_directories(f6_database_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(f6_database_lib PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

# Original database application
add_executable(f6_database
    src/main.c
)

target_link_libraries(f6_database PRIVATE
    f6_database_lib
)

# Command interpreter
add_executable(f6_interpreter
    ../interpreter/src/main.c
    ../interpreter/src/commands.c
    ../interpreter/src/lexer.c
    ../interpreter/src/parser.c
)

target_include_directories(f6_interpreter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter/src
)

target_link_libraries(f6_interpreter PRIVATE
    f6_database_lib
)

# Automated tests
enable_testing()

add_executable(test_database
    tests/test_database.c
)

target_link_libraries(test_database PRIVATE
    f6_database_lib
)

add_test(
    NAME database_tests
    COMMAND test_database
)
