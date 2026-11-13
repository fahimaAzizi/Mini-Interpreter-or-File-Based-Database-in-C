
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    CMD_INVALID = 0,
    CMD_SET,
    CMD_GET,
    CMD_UPDATE,
    CMD_DELETE,
    CMD_LIST,
    CMD_SAVE,
    CMD_HELP,
    CMD_EXIT,
    CMD_BACKUP,
    CMD_RESTORE
} CommandType;

typedef struct {
    CommandType type;
    char key[TOKEN_LENGTH];
    char value[TOKEN_LENGTH];
} ParsedCommand;

int parser_parse(const TokenList *tokens, ParsedCommand *command);

#endif
