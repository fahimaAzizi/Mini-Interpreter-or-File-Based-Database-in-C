#include "database.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>

#define MAX_RECORD_LINE_LENGTH \
    (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 2)

/* Validate database text fields. */
static int valid_text(const char *text, size_t max_length)
{
    if (text == NULL ||
        text[0] == '\0' ||
        strlen(text) >= max_length) {
        return 0;
    }

    for (const char *p = text; *p != '\0'; p++) {
        if (*p == '\t' || *p == '\n' || *p == '\r') {
            return 0;
        }
    }

    return 1;
}

/* FNV-1a hash function. */
static size_t hash_key(const char *key)
{
    size_t hash = (size_t)2166136261u;

    while (*key != '\0') {
        hash ^= (unsigned char)*key++;
        hash *= (size_t)16777619u;
    }

    return hash % INDEX_SIZE;
}

/* Find a record using the hash table. */
static int find_record(const Database *db, const char *key)
{
    size_t position = hash_key(key);

    for (size_t i = 0; i < INDEX_SIZE; i++) {
        size_t slot = (position + i) % INDEX_SIZE;
        size_t record_index = db->index[slot];

        if (record_index == INDEX_EMPTY) {
            return -1;
        }

        if (record_index < db->count &&
            strcmp(db->records[record_index].key, key) == 0) {
            return (int)record_index;
        }
    }

    return -1;
}

/* Insert a record index into the hash table. */
static int index_insert(Database *db, size_t record_index)
{
    size_t position = hash_key(db->records[record_index].key);

    for (size_t i = 0; i < INDEX_SIZE; i++) {
        size_t slot = (position + i) % INDEX_SIZE;

        if (db->index[slot] == INDEX_EMPTY) {
            db->index[slot] = record_index;
            return 1;
        }
    }

    return 0;
}

/* Rebuild the complete index. */
static int rebuild_index(Database *db)
{
    for (size_t i = 0; i < INDEX_SIZE; i++) {
        db->index[i] = INDEX_EMPTY;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (!index_insert(db, i)) {
            return 0;
        }
    }

    return 1;
}

void db_init(Database *db)
{
    if (db == NULL) {
        return;
    }

    db->count = 0;

    for (size_t i = 0; i < INDEX_SIZE; i++) {
        db->index[i] = INDEX_EMPTY;
    }
}

DBStatus db_create(Database *db, const char *key, const char *value)
{
    if (db == NULL ||
        !valid_text(key, MAX_KEY_LENGTH) ||
        !valid_text(value, MAX_VALUE_LENGTH)) {
        return DB_ERROR_INVALID_INPUT;
    }

    if (find_record(db, key) != -1) {
        return DB_ERROR_DUPLICATE_KEY;
    }

    if (db->count >= MAX_RECORDS) {
        return DB_ERROR_FULL;
    }

    size_t new_index = db->count;

    strcpy(db->records[new_index].key, key);
    strcpy(db->records[new_index].value, value);

    if (!index_insert(db, new_index)) {
        return DB_ERROR_FULL;
    }

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

    if (!rebuild_index(db)) {
        return DB_ERROR_FULL;
    }

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

#ifdef _WIN32
    if (!MoveFileExA(
            temp_filename,
            filename,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
        )) {
        remove(temp_filename);
        return DB_ERROR_FILE_WRITE;
    }
#else
    if (rename(temp_filename, filename) != 0) {
        remove(temp_filename);
        return DB_ERROR_FILE_WRITE;
    }
#endif

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

        if (length > 0 &&
            line[length - 1] != '\n' &&
            !feof(file)) {

            int ch;

            while ((ch = fgetc(file)) != '\n' && ch != EOF) {
                /* Discard oversized line. */
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