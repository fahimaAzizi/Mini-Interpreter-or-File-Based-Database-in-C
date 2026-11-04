#include "lexer.h"

#include <ctype.h>
#include <string.h>

int lexer_tokenize(const char *input, TokenList *result)
{
    if (input == NULL || result == NULL) {
        return 0;
    }

    result->count = 0;

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

        size_t token_length = 0;
        int quoted = 0;

        if (input[i] == '"') {
            quoted = 1;
            i++;
        }

        while (input[i] != '\0') {
            if (quoted) {
                if (input[i] == '"') {
                    i++;
                    break;
                }
            } else if (isspace((unsigned char)input[i])) {
                break;
            }

            if (token_length >= TOKEN_LENGTH - 1) {
                return 0;
            }

            result->tokens[result->count][token_length++] =
                input[i++];
        }

        if (quoted && input[i - 1] != '"') {
            return 0;
        }

        result->tokens[result->count][token_length] = '\0';
        result->count++;

        if (!quoted && input[i] != '\0' &&
            !isspace((unsigned char)input[i])) {
            return 0;
        }
    }

    return 1;
}