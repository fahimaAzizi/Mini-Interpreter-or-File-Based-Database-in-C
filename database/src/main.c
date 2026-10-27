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
        buffer[length - 1] = '\0';
    }

    return 1;
}

static void save_after_change(const Database *db)
{
    if (!db_save(db, DATABASE_FILE)) {
        printf("Warning: Could not save the database file.\n");
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

    if (db_load(&db, DATABASE_FILE)) {
        printf("Database loaded successfully.\n");
    } else {
        printf("Starting with an empty database.\n");
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

            if (db_create(&db, key, value)) {
                printf("Record created successfully.\n");
                save_after_change(&db);
            } else {
                printf("Create failed. Check the key, value, or capacity.\n");
            }
        } else if (strcmp(option, "2") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key))) {
                continue;
            }

            if (db_read(&db, key, value, sizeof(value))) {
                printf("Value: %s\n", value);
            } else {
                printf("Record not found.\n");
            }
        } else if (strcmp(option, "3") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key)) ||
                !read_input("Enter new value: ", value, sizeof(value))) {
                continue;
            }

            if (db_update(&db, key, value)) {
                printf("Record updated successfully.\n");
                save_after_change(&db);
            } else {
                printf("Update failed. Record not found or value invalid.\n");
            }
        } else if (strcmp(option, "4") == 0) {
            if (!read_input("Enter key: ", key, sizeof(key))) {
                continue;
            }

            if (db_delete(&db, key)) {
                printf("Record deleted successfully.\n");
                save_after_change(&db);
            } else {
                printf("Record not found.\n");
            }
        } else if (strcmp(option, "5") == 0) {
            list_records(&db);
        } else if (strcmp(option, "6") == 0) {
            if (db_save(&db, DATABASE_FILE)) {
                printf("Database saved successfully.\n");
            } else {
                printf("Could not save the database.\n");
            }
        } else if (strcmp(option, "7") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("Invalid option. Enter a number from 1 to 7.\n");
        }
    }

    return 0;
}