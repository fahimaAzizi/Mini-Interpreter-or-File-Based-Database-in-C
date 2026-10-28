#include "../src/database.h"

#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(condition)                                      \
    do {                                                      \
        if (condition) {                                      \
            tests_passed++;                                   \
            printf("[PASS] %s\n", #condition);                \
        } else {                                              \
            tests_failed++;                                   \
            printf("[FAIL] %s (line %d)\n", #condition,       \
                   __LINE__);                                 \
        }                                                     \
    } while (0)

static void test_create_and_read(void)
{
    Database db;
    char output[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "name", "Fahima"));
    CHECK(db.count == 1);
    CHECK(db_read(&db, "name", output, sizeof(output)));
    CHECK(strcmp(output, "Fahima") == 0);
}

static void test_duplicate_and_missing_keys(void)
{
    Database db;
    char output[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "city", "Quetta"));
    CHECK(!db_create(&db, "city", "Karachi"));
    CHECK(!db_read(&db, "missing", output, sizeof(output)));
    CHECK(!db_update(&db, "missing", "value"));
    CHECK(!db_delete(&db, "missing"));
    CHECK(db.count == 1);
}

static void test_update_and_delete(void)
{
    Database db;
    char output[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "language", "C"));
    CHECK(db_update(&db, "language", "C11"));
    CHECK(db_read(&db, "language", output, sizeof(output)));
    CHECK(strcmp(output, "C11") == 0);

    CHECK(db_delete(&db, "language"));
    CHECK(db.count == 0);
    CHECK(!db_read(&db, "language", output, sizeof(output)));
}

static void test_invalid_inputs(void)
{
    Database db;

    db_init(&db);

    CHECK(!db_create(&db, "", "value"));
    CHECK(!db_create(&db, "bad\tkey", "value"));
    CHECK(!db_create(&db, "key", "bad\nvalue"));
    CHECK(!db_create(&db, NULL, "value"));
    CHECK(!db_create(&db, "key", NULL));
    CHECK(db.count == 0);
}

static void test_capacity(void)
{
    Database db;
    char key[MAX_KEY_LENGTH];

    db_init(&db);

    for (int i = 0; i < MAX_RECORDS; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        CHECK(db_create(&db, key, "value"));
    }

    CHECK(db.count == MAX_RECORDS);
    CHECK(!db_create(&db, "extra", "value"));
}

static void test_save_and_load(void)
{
    Database original;
    Database loaded;
    char output[MAX_VALUE_LENGTH];
    const char *filename = "test_database.dat";

    db_init(&original);
    db_init(&loaded);

    CHECK(db_create(&original, "name", "Fahima"));
    CHECK(db_create(&original, "city", "Quetta"));
    CHECK(db_save(&original, filename));
    CHECK(db_load(&loaded, filename));
    CHECK(loaded.count == 2);

    CHECK(db_read(&loaded, "name", output, sizeof(output)));
    CHECK(strcmp(output, "Fahima") == 0);

    CHECK(db_read(&loaded, "city", output, sizeof(output)));
    CHECK(strcmp(output, "Quetta") == 0);

    remove(filename);
}

int main(void)
{
    printf("=== F6 Database Automated Tests ===\n\n");
test_create_and_read();
test_duplicate_and_missing_keys();
test_update_and_delete();
test_invalid_inputs();
test_capacity();
test_save_and_load();
test_corrupted_file();
    

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    return tests_failed == 0 ? 0 : 1;
}static void test_corrupted_file(void)
{
    Database db;
    char output[MAX_VALUE_LENGTH];

    db_init(&db);

    CHECK(db_create(&db, "original", "safe-data"));

    CHECK(!db_load(&db, "corrupted-test.dat"));

    CHECK(db.count == 1);

    CHECK(db_read(
        &db,
        "original",
        output,
        sizeof(output)
    ));

    CHECK(strcmp(output, "safe-data") == 0);
}