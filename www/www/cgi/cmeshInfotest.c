#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>




typedef struct {
    char name[20];
    char value[20];
    char state[5];
} User;

static int callback(void* data, int argc, char** argv, char** azColName) {
    int i;
    User* user = (User*)data;

    for (i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "name") == 0) {
            strcpy(user->name, argv[i]);
        }
        if (strcmp(azColName[i], "value") == 0) {
            strcpy(user->value, argv[i]);
printf("%s\n",user->value);
        }
        if (strcmp(azColName[i], "state") == 0) {
            strcpy(user->state, argv[i]);
        }
    }
printf("%s\n",user->name);

    return 0;
}

int main(int argc, char** argv) {
    sqlite3* db;
    char* errMsg = 0;
    int rc;
    char* sql;
    User user;

    rc = sqlite3_open("test.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sql = "SELECT name, value, state FROM meshInfo";

    rc = sqlite3_exec(db, sql, callback, &user, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

printf("%s\n",user.name);

    sqlite3_close(db);

    return 0;
}


