
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define CHECK(condition, description)             \
    do {                                          \
        tests_run++;                              \
        if (condition) {                          \
            tests_passed++;                       \
            printf("[PASS] %s\n", description);   \
        } else {                                  \
            printf("[FAIL] %s\n", description);   \
        }                                         \
    } while (0)

static void test_lexer(void)
{
    TokenList tokens;

    CHECK(
        lexer_tokenize("GET city", &tokens) &&
        tokens.count == 2 &&
        strcmp(tokens.tokens[0], "GET") == 0 &&
        strcmp(tokens.tokens[1], "city") == 0,
        "Tokenizes GET command"
    );

    CHECK(
        lexer_tokenize(
            "SET city \"Quetta Balochistan\"",
            &tokens
        ) &&
        tokens.count == 3 &&
        strcmp(tokens.tokens[2], "Quetta Balochistan") == 0,
        "Tokenizes quoted values"
    );

    CHECK(
        lexer_tokenize("", &tokens) &&
        tokens.count == 0,
        "Accepts empty input"
    );

    CHECK(
        !lexer_tokenize("ONE TWO THREE FOUR FIVE", &tokens),
        "Rejects too many tokens"
    );

    CHECK(
        !lexer_tokenize("SET city \"Quetta", &tokens),
        "Rejects unmatched quotation marks"
    );

    CHECK(
        lexer_tokenize(
            "SET message \"She said \\\"Hello\\\"\"",
            &tokens
        ) &&
        tokens.count == 3 &&
        strcmp(
            tokens.tokens[2],
            "She said \"Hello\""
        ) == 0,
        "Supports escaped quotation marks"
    );

    CHECK(
        lexer_tokenize(
            "SET path \"C:\\\\temp\"",
            &tokens
        ) &&
        strcmp(tokens.tokens[2], "C:\\temp") == 0,
        "Supports escaped backslashes"
    );

    CHECK(
        !lexer_tokenize(
            "SET key \"value\"extra",
            &tokens
        ),
        "Rejects text after closing quotation marks"
    );

    CHECK(
        !lexer_tokenize(
            "SET key \"bad\\nvalue\"",
            &tokens
        ),
        "Rejects unsupported escape sequences"
    );
}

static void check_command(
    const char *input,
    CommandType expected_type,
    const char *expected_key,
    const char *expected_value,
    const char *description
)
{
    TokenList tokens;
    ParsedCommand command;

    int result = lexer_tokenize(input, &tokens);

    if (result) {
        result = parser_parse(&tokens, &command);
    }

    CHECK(
        result &&
        command.type == expected_type &&
        strcmp(command.key, expected_key) == 0 &&
        strcmp(command.value, expected_value) == 0,
        description
    );
}

static void test_parser(void)
{
    check_command(
        "SET city \"Quetta\"",
        CMD_SET, "city", "Quetta",
        "Parses SET command"
    );

    check_command(
        "GET city",
        CMD_GET, "city", "",
        "Parses GET command"
    );

    check_command(
        "UPDATE city \"Quetta\"",
        CMD_UPDATE, "city", "Quetta",
        "Parses UPDATE command"
    );

    check_command(
        "DELETE city",
        CMD_DELETE, "city", "",
        "Parses DELETE command"
    );

    check_command(
        "LIST",
        CMD_LIST, "", "",
        "Parses LIST command"
    );

    check_command(
        "SAVE",
        CMD_SAVE, "", "",
        "Parses SAVE command"
    );

    check_command(
        "HELP",
        CMD_HELP, "", "",
        "Parses HELP command"
    );

    check_command(
        "EXIT",
        CMD_EXIT, "", "",
        "Parses EXIT command"
    );

    TokenList tokens;
    ParsedCommand command;

    lexer_tokenize("GET", &tokens);

    CHECK(
        !parser_parse(&tokens, &command),
        "Rejects missing GET argument"
    );

    lexer_tokenize("UNKNOWN city", &tokens);

    CHECK(
        !parser_parse(&tokens, &command),
        "Rejects unknown commands"
    );
}

int main(void)
{
    printf("\n========== F6 INTERPRETER TESTS ==========\n\n");

    test_lexer();
    test_parser();

    printf("\n==========================================\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("ALL INTERPRETER TESTS PASSED!\n");
        return 0;
    }

    printf("SOME TESTS FAILED.\n");
    return 1;
}
