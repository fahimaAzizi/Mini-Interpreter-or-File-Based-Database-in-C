#ifndef DATABASE_H
#define DATABASE_H

#include <stddef.h>

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

void db_init(Database *db);

int db_create(Database *db, const char *key, const char *value);

int db_read(
    const Database *db,
    const char *key,
    char *value_out,
    size_t output_size
);

int db_update(Database *db, const char *key, const char *new_value);

int db_delete(Database *db, const char *key);

int db_save(const Database *db, const char *filename);

int db_load(Database *db, const char *filename);

#endif