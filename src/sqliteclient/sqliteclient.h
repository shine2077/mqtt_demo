//
// Created by sun on 22-12-11.
//

#ifndef MQTT_DEMO_SQLITECLIENT_H
#define MQTT_DEMO_SQLITECLIENT_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstdarg>
#include <memory>
#include <vector>

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

    void instert_date(const std::string& key, const std::string &content);

    bool contains_key(const std::string& key);

    std::vector<std::string> keys();

    std::string getValue(const std::string &key);

    void removeKey(const std::string &key);

    void clearTable();

private:
    std::string formatSQL(const char* format, ...);


private:
    std::unique_ptr<SQLite::Database>  mDb;    ///< Database connection
    std::string table_;
};

#endif //MQTT_DEMO_SQLITECLIENT_H
