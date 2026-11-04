#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "database.h"

#define COMMAND_LENGTH 512
#define TOKEN_LENGTH 256
#define MAX_TOKENS 4

typedef enum {
    INTERPRETER_CONTINUE = 0,
    INTERPRETER_EXIT
} InterpreterResult;

InterpreterResult interpreter_execute(
    Database *db,
    const char *input,
    const char *filename
);

void interpreter_print_help(void);

#endif