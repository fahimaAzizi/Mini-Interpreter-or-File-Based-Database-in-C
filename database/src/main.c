
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RECORDS 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 100
#define DATABASE_FILE "database.dat"

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    size_t count;
} Database;

/* Find a record by key. */
static size_t db_find(const Database *db, const char *key)
{
    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            return i;
        }
    }

    return MAX_RECORDS;
}

/* Create a record. */
static int db_create(Database *db, const char *key,
                     const char *value)
{
    if (db == NULL || key == NULL || value == NULL ||
        key[0] == '\0' ||
        strlen(key) >= MAX_KEY_LENGTH ||
        strlen(value) >= MAX_VALUE_LENGTH ||
        strchr(key, '\t') != NULL ||
        strchr(key, '\n') != NULL ||
        strchr(value, '\t') != NULL ||
        strchr(value, '\n') != NULL ||
        strchr(key, '\r') != NULL ||
        strchr(value, '\r') != NULL ||
        db->count >= MAX_RECORDS ||
        db_find(db, key) != MAX_RECORDS) {
        return 0;
    }

    strcpy(db->records[db->count].key, key);
    strcpy(db->records[db->count].value, value);
    db->count++;

    return 1;
}

/* Read a record. */
static int db_read(const Database *db, const char *key,
                   char *output, size_t output_size)
{
    if (db == NULL || key == NULL || output == NULL ||
        output_size == 0 || key[0] == '\0') {
        return 0;
    }

    size_t index = db_find(db, key);

    if (index == MAX_RECORDS) {
        return 0;
    }

    size_t length = strlen(db->records[index].value);

    if (length >= output_size) {
        return 0;
    }

    memcpy(output, db->records[index].value, length + 1);
    return 1;
}

/* Update an existing record. */
static int db_update(Database *db, const char *key,
                     const char *value)
{
    if (db == NULL || key == NULL || value == NULL ||
        key[0] == '\0' ||
        strlen(value) >= MAX_VALUE_LENGTH ||
        strchr(value, '\t') != NULL ||
        strchr(value, '\n') != NULL ||
        strchr(value, '\r') != NULL) {
        return 0;
    }

    size_t index = db_find(db, key);

    if (index == MAX_RECORDS) {
        return 0;
    }

    strcpy(db->records[index].value, value);
    return 1;
}

/* Delete a record. */
static int db_delete(Database *db, const char *key)
{
    if (db == NULL || key == NULL || key[0] == '\0') {
        return 0;
    }

    size_t index = db_find(db, key);

    if (index == MAX_RECORDS) {
        return 0;
    }

    for (size_t i = index; i + 1 < db->count; i++) {
        db->records[i] = db->records[i + 1];
    }

    db->count--;
    return 1;
}

/* Save all records to disk. */
static int db_save(const Database *db, const char *filename)
{
    if (db == NULL || filename == NULL || filename[0] == '\0') {
        return 0;
    }

    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Could not open database file for saving");
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (fprintf(file, "%s\t%s\n",
                    db->records[i].key,
                    db->records[i].value) < 0) {
            fclose(file);
            return 0;
        }
    }

    if (fclose(file) != 0) {
        return 0;
    }

    return 1;
}

/* Load records into a temporary database first. */
static int db_load(Database *db, const char *filename)
{
    if (db == NULL || filename == NULL || filename[0] == '\0') {
        return 0;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return 0;
    }

    Database loaded = {0};
    char line[MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 3];

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t length = strlen(line);

        if (length == 0 || line[length - 1] != '\n') {
            if (!feof(file)) {
                fclose(file);
                fprintf(stderr, "Error: Database line too long.\n");
                return 0;
            }
        }

        if (length > 0 && line[length - 1] == '\n') {
            line[--length] = '\0';
        }

        if (length > 0 && line[length - 1] == '\r') {
            line[--length] = '\0';
        }

        char *separator = strchr(line, '\t');

        if (separator == NULL) {
            fclose(file);
            fprintf(stderr, "Error: Invalid database record.\n");
            return 0;
        }

        *separator = '\0';

        if (!db_create(&loaded, line, separator + 1)) {
            fclose(file);
            fprintf(stderr, "Error: Invalid or duplicate database record.\n");
            return 0;
        }
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        return 0;
    }

    *db = loaded;
    return 1;
}

/* Read a line safely and reject truncated input. */
static int read_input(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
    } else if (!feof(stdin)) {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF) {
        }

        printf("Error: Input is too long.\n");
        return -1;
    }

    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[--length] = '\0';
    }

    return 1;
}

/* Print every record. */
static void db_list(const Database *db)
{
    printf("\nTotal records: %zu\n", db->count);

    for (size_t i = 0; i < db->count; i++) {
        printf("%s = %s\n",
               db->records[i].key,
               db->records[i].value);
    }
}

int main(void)
{
    Database db = {0};
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    char output[MAX_VALUE_LENGTH];
    char choice[20];

    FILE *check = fopen(DATABASE_FILE, "r");

    if (check != NULL) {
        if (fclose(check) != 0 || !db_load(&db, DATABASE_FILE)) {
            fprintf(stderr, "Could not load database safely.\n");
            return 1;
        }
    }

    printf("=== F6 File-Based Database ===\n");

    for (;;) {
        printf("\n1. Create record\n");
        printf("2. Read record\n");
        printf("3. Update record\n");
        printf("4. Delete record\n");
        printf("5. List records\n");
        printf("6. Save database\n");
        printf("7. Exit\n");

        int status = read_input("Choose an option: ",
                                choice, sizeof(choice));

        if (status == 0) {
            break;
        }

        if (status < 0) {
            continue;
        }

        if (strcmp(choice, "1") == 0) {
            if (read_input("Enter key: ", key, sizeof(key)) != 1 ||
                read_input("Enter value: ", value, sizeof(value)) != 1) {
                printf("Input cancelled or invalid.\n");
                continue;
            }

            if (!db_create(&db, key, value)) {
                printf("Create failed. Check input, duplicates, or capacity.\n");
            } else if (db_save(&db, DATABASE_FILE)) {
                printf("Record created and saved.\n");
            } else {
                printf("Save failed. Record remains in memory.\n");
            }
        } else if (strcmp(choice, "2") == 0) {
            if (read_input("Enter key: ", key, sizeof(key)) != 1) {
                printf("Input cancelled or invalid.\n");
                continue;
            }

            if (db_read(&db, key, output, sizeof(output))) {
                printf("Value: %s\n", output);
            } else {
                printf("Record not found.\n");
            }
        } else if (strcmp(choice, "3") == 0) {
            if (read_input("Enter key: ", key, sizeof(key)) != 1 ||
                read_input("Enter new value: ", value, sizeof(value)) != 1) {
                printf("Input cancelled or invalid.\n");
                continue;
            }

            if (!db_update(&db, key, value)) {
                printf("Update failed. Record not found or value invalid.\n");
            } else if (db_save(&db, DATABASE_FILE)) {
                printf("Record updated and saved.\n");
            } else {
                printf("Save failed. Updated value remains in memory.\n");
            }
        } else if (strcmp(choice, "4") == 0) {
            if (read_input("Enter key: ", key, sizeof(key)) != 1) {
                printf("Input cancelled or invalid.\n");
                continue;
            }

            if (!db_delete(&db, key)) {
                printf("Record not found.\n");
            } else if (db_save(&db, DATABASE_FILE)) {
                printf("Record deleted and saved.\n");
            } else {
                printf("Save failed. Deletion remains in memory.\n");
            }
        } else if (strcmp(choice, "5") == 0) {
            db_list(&db);
        } else if (strcmp(choice, "6") == 0) {
            printf("%s\n", db_save(&db, DATABASE_FILE)
                ? "Database saved successfully."
                : "Could not save database.");
        } else if (strcmp(choice, "7") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("Invalid option. Enter a number from 1 to 7.\n");
        }
    }

    return 0;
}
