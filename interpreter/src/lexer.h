#ifndef LEXER_H
#define LEXER_H

#include "interpreter.h"

typedef struct {
    char tokens[MAX_TOKENS][TOKEN_LENGTH];
    size_t count;
} TokenList;

int lexer_tokenize(const char *input, TokenList *result);

#endif