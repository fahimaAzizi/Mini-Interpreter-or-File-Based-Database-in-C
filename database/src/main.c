
#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 100

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    size_t count;
} Database;

/* Add a new record to the database. */
int db_create(Database *db, const char *key, const char *value)
{
    if (db == NULL || key == NULL || value == NULL ||
        key[0] == '\0' ||
        strlen(key) >= MAX_KEY_LENGTH ||
        strlen(value) >= MAX_VALUE_LENGTH ||
        db->count >= MAX_RECORDS) {
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            return 0;
        }
    }

    strcpy(db->records[db->count].key, key);
    strcpy(db->records[db->count].value, value);
    db->count++;

    return 1;
}

/* Read a value using its key. */
int db_read(const Database *db, const char *key,
            char *output, size_t output_size)
{
    if (db == NULL || key == NULL || output == NULL ||
        output_size == 0 || key[0] == '\0') {
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            size_t length = strlen(db->records[i].value);

            if (length >= output_size) {
                return 0;
            }

            strcpy(output, db->records[i].value);
            return 1;
        }
    }

    return 0;
}

/* Update the value of an existing record. */
int db_update(Database *db, const char *key, const char *new_value)
{
    if (db == NULL || key == NULL || new_value == NULL ||
        key[0] == '\0' ||
        strlen(new_value) >= MAX_VALUE_LENGTH) {
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            strcpy(db->records[i].value, new_value);
            return 1;
        }
    }

    return 0;
}

/* Delete a record and close the gap in the array. */
int db_delete(Database *db, const char *key)
{
    if (db == NULL || key == NULL || key[0] == '\0') {
        return 0;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            for (size_t j = i; j + 1 < db->count; j++) {
                db->records[j] = db->records[j + 1];
            }

            db->count--;
            return 1;
        }
    }

    return 0;
}


int main(void)
{
    Database db = {0};
    char output[MAX_VALUE_LENGTH];

    if (!db_create(&db, "name", "Fahima")) {
        fprintf(stderr, "Error: Could not create record.\n");
        return 1;
    }

    if (db_read(&db, "name", output, sizeof(output))) {
        printf("Key: name\n");
        printf("Value: %s\n", output);
        printf("Total records: %zu\n", db.count);
    } else {
        fprintf(stderr, "Error: Could not read record.\n");
        return 1;
    }

    return 0;
}
