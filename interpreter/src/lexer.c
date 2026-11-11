
#include "lexer.h"

#include <ctype.h>
#include <string.h>

int lexer_tokenize(const char *input, TokenList *result)
{
    if (input == NULL || result == NULL) {
        return 0;
    }

    memset(result, 0, sizeof(*result));

    size_t i = 0;

    while (input[i] != '\0') {

        while (isspace((unsigned char)input[i])) {
            i++;
        }

        if (input[i] == '\0') {
            break;
        }

        if (result->count >= MAX_TOKENS) {
            return 0;
        }

        int quoted = 0;
        int closed = 0;
        size_t token_length = 0;

        if (input[i] == '"') {
            quoted = 1;
            i++;
        }

        while (input[i] != '\0') {

            char current = input[i];

            if (quoted) {

                if (current == '"') {
                    i++;
                    closed = 1;
                    break;
                }

                if (current == '\\') {
                    i++;

                    if (input[i] != '"' && input[i] != '\\') {
                        return 0;
                    }

                    current = input[i];
                }

                i++;
            } else {

                if (isspace((unsigned char)current)) {
                    break;
                }

                if (current == '"') {
                    return 0;
                }

                i++;
            }

            if (token_length >= TOKEN_LENGTH - 1) {
                return 0;
            }

            result->tokens[result->count][token_length++] = current;
        }

        if (quoted && !closed) {
            return 0;
        }

        if (quoted &&
            input[i] != '\0' &&
            !isspace((unsigned char)input[i])) {
            return 0;
        }

        result->tokens[result->count][token_length] = '\0';
        result->count++;
    }

    return 1;
}
