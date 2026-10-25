
#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 100
#define MAX_KEY_LENGTH 50
#define MAX_VALUE_LENGTH 100

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} Record;

int main(void)
{
    Record records[MAX_RECORDS];
    int count = 0;

    strcpy(records[count].key, "name");
    strcpy(records[count].value, "Fahima");
    count++;

    printf("Key: %s\n", records[0].key);
    printf("Value: %s\n", records[0].value);
    printf("Total records: %d\n", count);

    return 0;
}
