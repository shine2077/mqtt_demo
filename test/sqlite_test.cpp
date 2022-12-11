//
// Created by sun on 22-12-11.
//
#include "../src/sqliteclient/sqliteclient.h"

int main()
{
    SQLiteClient* sqlite_client = new SQLiteClient();
    sqlite_client->open("./test", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    sqlite_client->createTable("test");

    delete sqlite_client;
    return 0;
}
