
#include "database.h"

#include <stdio.h>
#include <string.h>

#define MAX_RECORD_LINE_LENGTH \
    (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 2)

static int valid_text(const char *text, size_t max_length)
{
    if (text == NULL || strlen(text) >= max_length) {
        return 0;
    }

    for (const char *p = text; *p != '\0'; p++) {
        if (*p == '\t' || *p == '\n' || *p == '\r') {
            return 0;
        }
    }

    return 1;
}

static int find_record(const Database *db, const char *key)
{
    for (size_t i = 0; i < db->count; i++) {
        if (strcmp(db->records[i].key, key) == 0) {
            return (int)i;
        }
    }

    return -1;
}

void db_init(Database *db)
{
    if (db != NULL) {
        db->count = 0;
    }
}

DBStatus db_create(Database *db, const char *key, const char *value)
{
    if (db == NULL ||
        !valid_text(key, MAX_KEY_LENGTH) ||
        key[0] == '\0' ||
        !valid_text(value, MAX_VALUE_LENGTH)) {
        return DB_ERROR_INVALID_INPUT;
    }

    if (find_record(db, key) != -1) {
        return DB_ERROR_DUPLICATE_KEY;
    }

    if (db->count >= MAX_RECORDS) {
        return DB_ERROR_FULL;
    }

    Record *record = &db->records[db->count];

    strcpy(record->key, key);
    strcpy(record->value, value);

    db->count++;

    return DB_SUCCESS;
}

DBStatus db_read(
    const Database *db,
    const char *key,
    char *value_out,
    size_t output_size
)
{
    if (db == NULL || key == NULL ||
        value_out == NULL || output_size == 0) {
        return DB_ERROR_INVALID_INPUT;
    }

    int index = find_record(db, key);

    if (index == -1) {
        return DB_ERROR_NOT_FOUND;
    }

    size_t length = strlen(db->records[index].value);

    if (length >= output_size) {
        return DB_ERROR_BUFFER_TOO_SMALL;
    }

    strcpy(value_out, db->records[index].value);

    return DB_SUCCESS;
}

DBStatus db_update(Database *db, const char *key, const char *new_value)
{
    if (db == NULL || key == NULL ||
        !valid_text(new_value, MAX_VALUE_LENGTH)) {
        return DB_ERROR_INVALID_INPUT;
    }

    int index = find_record(db, key);

    if (index == -1) {
        return DB_ERROR_NOT_FOUND;
    }

    strcpy(db->records[index].value, new_value);

    return DB_SUCCESS;
}

DBStatus db_delete(Database *db, const char *key)
{
    if (db == NULL || key == NULL) {
        return DB_ERROR_INVALID_INPUT;
    }

    int index = find_record(db, key);

    if (index == -1) {
        return DB_ERROR_NOT_FOUND;
    }

    for (size_t i = (size_t)index; i + 1 < db->count; i++) {
        db->records[i] = db->records[i + 1];
    }

    db->count--;

    return DB_SUCCESS;
}

DBStatus db_save(const Database *db, const char *filename)
{
    if (db == NULL || filename == NULL || filename[0] == '\0') {
        return DB_ERROR_INVALID_INPUT;
    }

    char temp_filename[512];

    int length = snprintf(
        temp_filename,
        sizeof(temp_filename),
        "%s.tmp",
        filename
    );

    if (length < 0 || (size_t)length >= sizeof(temp_filename)) {
        return DB_ERROR_INVALID_INPUT;
    }

    FILE *file = fopen(temp_filename, "w");

    if (file == NULL) {
        return DB_ERROR_FILE_OPEN;
    }

    int success = 1;

    for (size_t i = 0; i < db->count; i++) {
        if (fprintf(
                file,
                "%s\t%s\n",
                db->records[i].key,
                db->records[i].value
            ) < 0) {
            success = 0;
            break;
        }
    }

    if (fclose(file) != 0) {
        success = 0;
    }

    if (!success) {
        remove(temp_filename);
        return DB_ERROR_FILE_WRITE;
    }

    if (rename(temp_filename, filename) != 0) {
        remove(temp_filename);
        return DB_ERROR_FILE_WRITE;
    }

    return DB_SUCCESS;
}

DBStatus db_load(Database *db, const char *filename)
{
    if (db == NULL || filename == NULL || filename[0] == '\0') {
        return DB_ERROR_INVALID_INPUT;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return DB_ERROR_FILE_OPEN;
    }

    Database loaded;
    db_init(&loaded);

    char line[MAX_RECORD_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) != NULL) {
        size_t length = strlen(line);

        if (length > 0 && line[length - 1] != '\n' &&
            !feof(file)) {
            int ch;

            while ((ch = fgetc(file)) != '\n' && ch != EOF) {
                /* Discard the rest of the oversized line. */
            }

            fclose(file);
            return DB_ERROR_CORRUPTED_FILE;
        }

        line[strcspn(line, "\r\n")] = '\0';

        char *separator = strchr(line, '\t');

        if (separator == NULL) {
            fclose(file);
            return DB_ERROR_CORRUPTED_FILE;
        }

        *separator = '\0';

        DBStatus status = db_create(
            &loaded,
            line,
            separator + 1
        );

        if (status != DB_SUCCESS) {
            fclose(file);
            return DB_ERROR_CORRUPTED_FILE;
        }
    }

    if (ferror(file)) {
        fclose(file);
        return DB_ERROR_FILE_READ;
    }

    if (fclose(file) != 0) {
        return DB_ERROR_FILE_READ;
    }

    *db = loaded;

    return DB_SUCCESS;
}

const char *db_status_message(DBStatus status)
{
    switch (status) {
        case DB_SUCCESS:
            return "Operation completed successfully.";

        case DB_ERROR_INVALID_INPUT:
            return "Invalid input.";

        case DB_ERROR_DUPLICATE_KEY:
            return "A record with this key already exists.";

        case DB_ERROR_NOT_FOUND:
            return "Record not found.";

        case DB_ERROR_FULL:
            return "Database is full.";

        case DB_ERROR_FILE_OPEN:
            return "Could not open the database file.";

        case DB_ERROR_FILE_WRITE:
            return "Could not write or replace the database file.";

        case DB_ERROR_FILE_READ:
            return "Could not read the database file.";

        case DB_ERROR_CORRUPTED_FILE:
            return "Database file is corrupted.";

        case DB_ERROR_BUFFER_TOO_SMALL:
            return "Output buffer is too small.";

        default:
            return "Unknown database error.";
    }
}
