#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>

#define MAX_RECORDS 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 100

typedef enum {
    DB_SUCCESS = 0,
    DB_ERROR_INVALID_INPUT,
    DB_ERROR_DUPLICATE_KEY,
    DB_ERROR_NOT_FOUND,
    DB_ERROR_FULL,
    DB_ERROR_FILE_OPEN,
    DB_ERROR_FILE_WRITE,
    DB_ERROR_FILE_READ,
    DB_ERROR_CORRUPTED_FILE,
    DB_ERROR_BUFFER_TOO_SMALL
} DBStatus;

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    size_t count;
} Database;

void db_init(Database *db);

DBStatus db_create(Database *db, const char *key, const char *value);

DBStatus db_read(
    const Database *db,
    const char *key,
    char *value_out,
    size_t output_size
);

DBStatus db_update(Database *db, const char *key, const char *new_value);

DBStatus db_delete(Database *db, const char *key);

DBStatus db_save(const Database *db, const char *filename);

DBStatus db_load(Database *db, const char *filename);

const char *db_status_message(DBStatus status);

#endif