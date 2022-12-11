//
// Created by sun on 22-12-11.
//

#ifndef MQTT_DEMO_SQLITECLIENT_H
#define MQTT_DEMO_SQLITECLIENT_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

class SQLiteClient
{
public:
    SQLiteClient() : mDb(nullptr) {};

    // SQLiteClient is non-copyable
    SQLiteClient(const SQLiteClient &) = delete;
    SQLiteClient& operator=(const SQLiteClient&) = delete;

    ~SQLiteClient() = default;

    void open(const std::string &filename, const int mode = SQLite::OPEN_READONLY);
    //create table.if table is exited and clean flag is true, recreate a new table. else if
    //clean flag is false, maintain this.
    void createTable(const std::string &tablename, bool clean = false);


private:
    std::unique_ptr<SQLite::Database>  mDb;    ///< Database connection
};

#endif //MQTT_DEMO_SQLITECLIENT_H
