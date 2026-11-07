#include "interpreter.h"
#include "parser.h"

#include <stdio.h>

static void print_error(DBStatus status)
{
    printf("ERROR: %s\n", db_status_message(status));
}

static void save_after_change(
    const Database *db,
    const char *filename
)
{
    DBStatus status = db_save(db, filename);

    if (status != DB_SUCCESS) {
        printf("WARNING: Saving failed: %s\n",
               db_status_message(status));
    } else {
        printf("Saved successfully.\n");
    }
}

static void list_records(const Database *db)
{
    if (db->count == 0) {
        printf("Database is empty.\n");
        return;
    }

    printf("\n--- Database Records ---\n");

    for (size_t i = 0; i < db->count; i++) {
        printf("%zu. %s = %s\n",
               i + 1,
               db->records[i].key,
               db->records[i].value);
    }

    printf("Total records: %zu\n", db->count);
}

void interpreter_print_help(void)
{
    printf("\n========== F6 COMMAND REFERENCE ==========\n");
    printf("SET key \"value\"       Create a record\n");
    printf("GET key                Read a record\n");
    printf("UPDATE key \"value\"    Update a record\n");
    printf("DELETE key             Delete a record\n");
    printf("LIST                   List all records\n");
    printf("SAVE                   Save database\n");
    printf("HELP                   Show help\n");
    printf("EXIT                   Exit interpreter\n");
    printf("==========================================\n\n");
}

InterpreterResult interpreter_execute(
    Database *db,
    const char *input,
    const char *filename
)
{
    if (db == NULL || input == NULL || filename == NULL) {
        printf("ERROR: Invalid interpreter input.\n");
        return INTERPRETER_CONTINUE;
    }

    TokenList tokens;

    if (!lexer_tokenize(input, &tokens)) {
        printf("ERROR: Invalid command syntax.\n");
        return INTERPRETER_CONTINUE;
    }

    if (tokens.count == 0) {
        return INTERPRETER_CONTINUE;
    }

    ParsedCommand command;

    if (!parser_parse(&tokens, &command)) {
        printf("ERROR: Invalid command. Type HELP.\n");
        return INTERPRETER_CONTINUE;
    }

    DBStatus status;
    char value[MAX_VALUE_LENGTH];

    switch (command.type) {
        case CMD_SET:
            status = db_create(db, command.key, command.value);
            if (status == DB_SUCCESS) {
                printf("OK: Record created.\n");
                save_after_change(db, filename);
            } else {
                print_error(status);
            }
            break;

        case CMD_GET:
            status = db_read(db, command.key, value, sizeof(value));
            if (status == DB_SUCCESS) {
                printf("%s\n", value);
            } else {
                print_error(status);
            }
            break;

        case CMD_UPDATE:
            status = db_update(db, command.key, command.value);
            if (status == DB_SUCCESS) {
                printf("OK: Record updated.\n");
                save_after_change(db, filename);
            } else {
                print_error(status);
            }
            break;

        case CMD_DELETE:
            status = db_delete(db, command.key);
            if (status == DB_SUCCESS) {
                printf("OK: Record deleted.\n");
                save_after_change(db, filename);
            } else {
                print_error(status);
            }
            break;

        case CMD_LIST:
            list_records(db);
            break;

        case CMD_SAVE:
            status = db_save(db, filename);
            if (status == DB_SUCCESS) {
                printf("OK: Database saved.\n");
            } else {
                print_error(status);
            }
            break;

        case CMD_HELP:
            interpreter_print_help();
            break;

        case CMD_EXIT:
            printf("Goodbye!\n");
            return INTERPRETER_EXIT;

        default:
            printf("ERROR: Unsupported command.\n");
            break;
    }

    return INTERPRETER_CONTINUE;
}