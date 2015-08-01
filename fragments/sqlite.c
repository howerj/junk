/* Modified from <https://www.sqlite.org/quickstart.html> from code placed
 * in the public domain <https://www.sqlite.org/copyright.html> */
#include <stdio.h>
#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
        int i;
        for (i = 0; i < argc; i++)
                printf("(%s %s)\n", azColName[i], argv[i] ? argv[i] : "NULL");
        printf("\n");
        return 0;
}

int main(int argc, char **argv)
{
        sqlite3 *db;
        char *zErrMsg = 0;

        if (argc != 3)
                return fprintf(stderr, "usage: %s DATABASE SQL-STATEMENT\n", argv[0]), -1;

        if(sqlite3_open(argv[1], &db)) {
                fprintf(stderr, "(error \"cannot open database: %s\")\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return 1;
        }
        if(sqlite3_exec(db, argv[2], callback, 0, &zErrMsg) != SQLITE_OK) {
                fprintf(stderr, "(sql-error \"%s\")\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }
        sqlite3_close(db);
        return 0;
}
