
#include "parser.h"

#include <string.h>

static int copy_token(char *destination, const char *source)
{
    size_t length = strlen(source);

    if (length == 0 || length >= TOKEN_LENGTH) {
        return 0;
    }

    memcpy(destination, source, length + 1);
    return 1;
}

int parser_parse(const TokenList *tokens, ParsedCommand *command)
{
    if (tokens == NULL || command == NULL || tokens->count == 0) {
        return 0;
    }

    memset(command, 0, sizeof(*command));

    const char *operation = tokens->tokens[0];

    if (strcmp(operation, "SET") == 0) {
        command->type = CMD_SET;

        if (tokens->count != 3) return 0;

        return copy_token(command->key, tokens->tokens[1]) &&
               copy_token(command->value, tokens->tokens[2]);
    }

    if (strcmp(operation, "GET") == 0) {
        command->type = CMD_GET;

        if (tokens->count != 2) return 0;

        return copy_token(command->key, tokens->tokens[1]);
    }

    if (strcmp(operation, "UPDATE") == 0) {
        command->type = CMD_UPDATE;

        if (tokens->count != 3) return 0;

        return copy_token(command->key, tokens->tokens[1]) &&
               copy_token(command->value, tokens->tokens[2]);
    }

    if (strcmp(operation, "DELETE") == 0) {
        command->type = CMD_DELETE;

        if (tokens->count != 2) return 0;

        return copy_token(command->key, tokens->tokens[1]);
    }

    if (strcmp(operation, "LIST") == 0) {
        command->type = CMD_LIST;
        return tokens->count == 1;
    }

    if (strcmp(operation, "SAVE") == 0) {
        command->type = CMD_SAVE;
        return tokens->count == 1;
    }

    if (strcmp(operation, "BACKUP") == 0) {
        command->type = CMD_BACKUP;
        return tokens->count == 1;
    }

    if (strcmp(operation, "RESTORE") == 0) {
        command->type = CMD_RESTORE;
        return tokens->count == 1;
    }

    if (strcmp(operation, "HELP") == 0) {
        command->type = CMD_HELP;
        return tokens->count == 1;
    }

    if (strcmp(operation, "EXIT") == 0) {
        command->type = CMD_EXIT;
        return tokens->count == 1;
    }

    return 0;
}
