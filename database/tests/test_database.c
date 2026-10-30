
#include "database.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(condition)                                      \
    do {                                                      \
        tests_run++;                                          \
        if (!(condition)) {                                   \
            tests_failed++;                                   \
            printf("FAILED: %s (line %d)\n",                  \
                   #condition, __LINE__);                     \
        }                                                     \
    } while (0)

static void test_create_and_read(void)
{
    Database db;
    char value[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "name", "Fahima") == DB_SUCCESS);
    CHECK(db.count == 1);

    CHECK(db_read(&db, "name", value, sizeof(value)) == DB_SUCCESS);
    CHECK(strcmp(value, "Fahima") == 0);
}

static void test_duplicate_and_missing_keys(void)
{
    Database db;
    char value[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "name", "Fahima") == DB_SUCCESS);

    CHECK(db_create(&db, "name", "Another") ==
          DB_ERROR_DUPLICATE_KEY);

    CHECK(db_read(&db, "missing", value, sizeof(value)) ==
          DB_ERROR_NOT_FOUND);

    CHECK(db_update(&db, "missing", "value") ==
          DB_ERROR_NOT_FOUND);

    CHECK(db_delete(&db, "missing") == DB_ERROR_NOT_FOUND);
    CHECK(db.count == 1);
}

static void test_update_and_delete(void)
{
    Database db;
    char value[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "language", "C") == DB_SUCCESS);
    CHECK(db_update(&db, "language", "C11") == DB_SUCCESS);

    CHECK(db_read(&db, "language", value, sizeof(value)) ==
          DB_SUCCESS);
    CHECK(strcmp(value, "C11") == 0);

    CHECK(db_delete(&db, "language") == DB_SUCCESS);
    CHECK(db.count == 0);

    CHECK(db_read(&db, "language", value, sizeof(value)) ==
          DB_ERROR_NOT_FOUND);
}

static void test_invalid_inputs(void)
{
    Database db;

    db_init(&db);

    CHECK(db_create(&db, "", "value") == DB_ERROR_INVALID_INPUT);
    CHECK(db_create(&db, "key", "") == DB_ERROR_INVALID_INPUT);
    CHECK(db_create(&db, "bad\tkey", "value") ==
          DB_ERROR_INVALID_INPUT);
    CHECK(db_create(&db, "key", "bad\nvalue") ==
          DB_ERROR_INVALID_INPUT);
    CHECK(db_create(&db, NULL, "value") == DB_ERROR_INVALID_INPUT);
    CHECK(db_create(&db, "key", NULL) == DB_ERROR_INVALID_INPUT);

    CHECK(db.count == 0);
}

static void test_database_capacity(void)
{
    Database db;
    char key[MAX_KEY_LENGTH];

    db_init(&db);

    for (int i = 0; i < MAX_RECORDS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        CHECK(db_create(&db, key, "value") == DB_SUCCESS);
    }

    CHECK(db.count == MAX_RECORDS);
    CHECK(db_create(&db, "extra", "value") == DB_ERROR_FULL);
}

static void test_save_and_load(void)
{
    Database original;
    Database loaded;
    char value[MAX_VALUE_LENGTH];

    const char *filename = "test_database.dat";

    db_init(&original);
    db_init(&loaded);

    CHECK(db_create(&original, "name", "Fahima") == DB_SUCCESS);
    CHECK(db_create(&original, "language", "C11") == DB_SUCCESS);

    CHECK(db_save(&original, filename) == DB_SUCCESS);
    CHECK(db_load(&loaded, filename) == DB_SUCCESS);

    CHECK(loaded.count == 2);

    CHECK(db_read(&loaded, "name", value, sizeof(value)) ==
          DB_SUCCESS);
    CHECK(strcmp(value, "Fahima") == 0);

    CHECK(db_read(&loaded, "language", value, sizeof(value)) ==
          DB_SUCCESS);
    CHECK(strcmp(value, "C11") == 0);

    remove(filename);
}

static void test_corrupted_file(void)
{
    Database db;
    char value[MAX_VALUE_LENGTH];

    const char *filename = "test_corrupted_database.dat";

    db_init(&db);

    CHECK(db_create(&db, "safe", "data") == DB_SUCCESS);

    FILE *file = fopen(filename, "w");

    CHECK(file != NULL);

    if (file == NULL) {
        return;
    }

    CHECK(fputs("this is not a valid database record\n", file) >= 0);
    CHECK(fclose(file) == 0);

    CHECK(db_load(&db, filename) == DB_ERROR_CORRUPTED_FILE);

    /* A failed load must not destroy existing in-memory data. */
    CHECK(db.count == 1);

    CHECK(db_read(&db, "safe", value, sizeof(value)) == DB_SUCCESS);
    CHECK(strcmp(value, "data") == 0);

    remove(filename);
}

int main(void)
{
    printf("Running database tests...\n\n");

    test_create_and_read();
    test_duplicate_and_missing_keys();
    test_update_and_delete();
    test_invalid_inputs();
    test_database_capacity();
    test_save_and_load();
    test_corrupted_file();

    printf("\nTests run: %d\n", tests_run);
    printf("Tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    }

    printf("\nSome tests failed.\n");
    return 1;
}
