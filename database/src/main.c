
#include "database.h"

#include <stdio.h>
#include <string.h>

#define DATABASE_FILE "database.dat"
#define INPUT_LENGTH 256

static int read_input(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
    } else {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Clear remaining input. */
        }

        if (!feof(stdin)) {
            printf("Input is too long. Please try again.\n");
            return 0;
        }
    }

    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[--length] = '\0';
    }

    if (buffer[0] == '\0') {
        printf("Input cannot be empty. Please try again.\n");
        return 0;
    }

    return 1;
}

static void report_status(DBStatus status)
{
    if (status != DB_SUCCESS) {
        printf("Error: %s\n", db_status_message(status));
    } else {
        printf("%s\n", db_status_message(status));
    }
}

static void save_after_change(const Database *db)
{
    DBStatus status = db_save(db, DATABASE_FILE);

    if (status != DB_SUCCESS) {
        report_status(status);
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
        printf(
            "%zu. Key: %s | Value: %s\n",
            i + 1,
            db->records[i].key,
            db->records[i].value
        );
    }
}

int main(void)
{
    Database db;
    db_init(&db);

    DBStatus load_status = db_load(&db, DATABASE_FILE);

    if (load_status == DB_SUCCESS) {
        printf("Database loaded successfully.\n");
    } else if (load_status == DB_ERROR_FILE_OPEN) {
        printf("No existing database found. Starting a new database.\n");
    } else {
        report_status(load_status);
        printf("Existing database was not loaded.\n");
    }

    char option[INPUT_LENGTH];
    char key[INPUT_LENGTH];
    char value[INPUT_LENGTH];

    for (;;) {
        printf("\n=== F6 File-Based Database ===\n");
        printf("1. Create record\n");
        printf("2. Read record\n");
        printf("3. Update record\n");
        printf("4. Delete record\n");
        printf("5. List records\n");
        printf("6. Save database\n");
        printf("7. Exit\n");

        if (!read_input("Choose an option: ", option, sizeof(option))) {
            continue;
        }

        if (strcmp(option, "1") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key)) ||
                !read_input("Enter value: ", value, sizeof(value))) {
                continue;
            }

            DBStatus status = db_create(&db, key, value);

            if (status == DB_SUCCESS) {
                report_status(status);
                save_after_change(&db);
            } else {
                report_status(status);
            }
        } else if (strcmp(option, "2") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key))) {
                continue;
            }

            DBStatus status = db_read(
                &db,
                key,
                value,
                sizeof(value)
            );

            if (status == DB_SUCCESS) {
                printf("Value: %s\n", value);
            } else {
                report_status(status);
            }
        } else if (strcmp(option, "3") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key)) ||
                !read_input("Enter new value: ", value, sizeof(value))) {
                continue;
            }

            DBStatus status = db_update(&db, key, value);

            if (status == DB_SUCCESS) {
                report_status(status);
                save_after_change(&db);
            } else {
                report_status(status);
            }
        } else if (strcmp(option, "4") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key))) {
                continue;
            }

            DBStatus status = db_delete(&db, key);

            if (status == DB_SUCCESS) {
                report_status(status);
                save_after_change(&db);
            } else {
                report_status(status);
            }
        } else if (strcmp(option, "5") == 0) {
            list_records(&db);
        } else if (strcmp(option, "6") == 0) {
            report_status(db_save(&db, DATABASE_FILE));
        } else if (strcmp(option, "7") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("Invalid option. Enter a number from 1 to 7.\n");
        }
    }

    return 0;
}
