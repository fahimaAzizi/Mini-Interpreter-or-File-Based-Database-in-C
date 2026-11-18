
#include "interpreter.h"
#include "parser.h"

#include <stdio.h>

#define BACKUP_FILENAME_LENGTH 512

static void print_error(DBStatus status)
{
    printf("ERROR: %s\n", db_status_message(status));
}

static int make_backup_filename(
    const char *filename,
    char *backup_filename,
    size_t buffer_size
)
{
    int length = snprintf(
        backup_filename,
        buffer_size,
        "%s.bak",
        filename
    );

    return length >= 0 && (size_t)length < buffer_size;
}

static void save_after_change(
    const Database *db,
    const char *filename
)
{
    if (db->transaction_active) {
        printf("Transaction active: change not saved yet.\n");
        return;
    }

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

static void create_backup(const Database *db, const char *filename)
{
    char backup_filename[BACKUP_FILENAME_LENGTH];

    if (!make_backup_filename(
            filename,
            backup_filename,
            sizeof(backup_filename))) {
        printf("ERROR: Backup filename is too long.\n");
        return;
    }

    DBStatus status = db_save(db, backup_filename);

    if (status == DB_SUCCESS) {
        printf("OK: Backup created: %s\n", backup_filename);
    } else {
        print_error(status);
    }
}

static void restore_backup(
    Database *db,
    const char *filename
)
{
    char backup_filename[BACKUP_FILENAME_LENGTH];

    if (!make_backup_filename(
            filename,
            backup_filename,
            sizeof(backup_filename))) {
        printf("ERROR: Backup filename is too long.\n");
        return;
    }

    Database restored;
    db_init(&restored);

    DBStatus status = db_load(&restored, backup_filename);

    if (status != DB_SUCCESS) {
        printf("ERROR: Backup could not be loaded: ");
        print_error(status);
        return;
    }

    status = db_save(&restored, filename);

    if (status != DB_SUCCESS) {
        printf("ERROR: Could not save restored database: ");
        print_error(status);
        return;
    }

    *db = restored;

    printf("OK: Database restored from %s\n", backup_filename);
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
    printf("BACKUP                 Create database backup\n");
    printf("RESTORE                Restore database backup\n");
    printf("BEGIN                  Start transaction\n");
    printf("COMMIT                 Save transaction changes\n");
    printf("ROLLBACK               Undo transaction changes\n");
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
            if (db->transaction_active) {
                printf("ERROR: Cannot SAVE during a transaction. Use COMMIT or ROLLBACK.\n");
                break;
            }

            status = db_save(db, filename);
            if (status == DB_SUCCESS) {
                printf("OK: Database saved.\n");
            } else {
                print_error(status);
            }
            break;

        case CMD_BACKUP:
            if (db->transaction_active) {
                printf("ERROR: Cannot BACKUP during a transaction.\n");
                break;
            }

            create_backup(db, filename);
            break;

        case CMD_RESTORE:
            if (db->transaction_active) {
                printf("ERROR: Cannot RESTORE during a transaction.\n");
                break;
            }

            restore_backup(db, filename);
            break;

        case CMD_BEGIN:
            status = db_transaction_begin(db);
            if (status == DB_SUCCESS) {
                printf("OK: Transaction started.\n");
            } else {
                print_error(status);
            }
            break;

        case CMD_COMMIT:
            status = db_transaction_commit(db, filename);
            if (status == DB_SUCCESS) {
                printf("OK: Transaction committed and saved.\n");
            } else {
                print_error(status);
            }
            break;

        case CMD_ROLLBACK:
            status = db_transaction_rollback(db);
            if (status == DB_SUCCESS) {
                printf("OK: Transaction rolled back.\n");
            } else {
                print_error(status);
            }
            break;

        case CMD_HELP:
            interpreter_print_help();
            break;

        case CMD_EXIT:
            if (db->transaction_active) {
                printf("ERROR: Active transaction. Use COMMIT or ROLLBACK before EXIT.\n");
                break;
            }

            printf("Goodbye!\n");
            return INTERPRETER_EXIT;

        default:
            printf("ERROR: Unsupported command.\n");
            break;
    }

    return INTERPRETER_CONTINUE;
}
