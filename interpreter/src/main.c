#include "interpreter.h"

#include <stdio.h>
#include <string.h>

#define DATABASE_FILE "database.dat"

int main(void)
{
    Database db;
    db_init(&db);

    DBStatus status = db_load(&db, DATABASE_FILE);

    if (status == DB_SUCCESS) {
        printf("F6 database loaded successfully.\n");
    } else if (status == DB_ERROR_FILE_OPEN) {
        printf("No existing database found. Starting fresh.\n");
    } else {
        printf(
            "Database loading error: %s\n",
            db_status_message(status)
        );

        return 1;
    }

    printf("\n");
    printf("====================================\n");
    printf("     F6 DATABASE COMMAND ENGINE    \n");
    printf("====================================\n");
    printf("Type HELP to see available commands.\n\n");

    char input[COMMAND_LENGTH];

    for (;;) {
        printf("F6> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nGoodbye!\n");
            break;
        }

        size_t length = strlen(input);

        if (length > 0 && input[length - 1] == '\n') {
            input[--length] = '\0';
        } else if (!feof(stdin)) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF) {
                /* Discard excess input. */
            }

            printf("ERROR: Command is too long.\n");
            continue;
        }

        if (length > 0 && input[length - 1] == '\r') {
            input[--length] = '\0';
        }

        InterpreterResult result = interpreter_execute(
            &db,
            input,
            DATABASE_FILE
        );

        if (result == INTERPRETER_EXIT) {
            break;
        }
    }

    return 0;
}