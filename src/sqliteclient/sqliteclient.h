//
// Created by sun on 22-12-14.
//

#ifndef MQTT_DEMO_SQLITECLIENTB_H
#define MQTT_DEMO_SQLITECLIENTB_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstdarg>
#include <memory>
#include <vector>

#include <sqlite3.h>

#include "mqtt/async_client.h"

// Number of key/values to place in database
static int FLAGS_num = 1000000;

// Number of read operations to do.  If negative, do FLAGS_num reads.
static int FLAGS_reads = -1;

// Size of each value
static int FLAGS_value_size = 100;

// Print histogram of operation timings
static bool FLAGS_histogram = false;

// Arrange to generate values that shrink to this fraction of
// their original size after compression
static double FLAGS_compression_ratio = 0.5;

// Page size. Default 1 KB.
static int FLAGS_page_size = 1024;

// Number of pages.
// Default cache size = FLAGS_page_size * FLAGS_num_pages = 4 MB.
static int FLAGS_num_pages = 4096;

// If true, do not destroy the existing database.  If you set this
// flag and also specify a benchmark that wants a fresh database, that
// benchmark will fail.
static bool FLAGS_use_existing_db = false;

// If true, the SQLite table has ROWIDs.
static bool FLAGS_use_rowids = false;

// If true, we allow batch writes to occur
static bool FLAGS_transaction = true;

// If true, we enable Write-Ahead Logging
static bool FLAGS_WAL_enabled = false;

// Use the db with the following name.
static const char* FLAGS_db = nullptr;

inline static void ErrorCheck(int status) {
    if (status != SQLITE_OK) {
        std::fprintf(stderr, "sqlite3 error: status = %d\n", status);
        //std::exit(1);
        throw mqtt::persistence_exception();
    }
}

inline static void ExecErrorCheck(int status, char* err_msg) {
    if (status != SQLITE_OK) {
        std::fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        //std::exit(1);
        throw mqtt::persistence_exception();
    }
}

class SQLiteClient
{
public:
    SQLiteClient() : db_(nullptr) {};

    // SQLiteClient is non-copyable
    SQLiteClient(const SQLiteClient &) = delete;
    SQLiteClient& operator=(const SQLiteClient&) = delete;

    ~SQLiteClient();

    void open(const std::string &filename);
    //create table.if table is exited and clean flag is true, recreate a new table. else if
    //clean flag is false, maintain this.
    void createTable(const std::string &tablename, bool clean = false);

    void instert_date(const std::string& key, const std::string &value);

    bool contains_key(const std::string& key);

    std::vector<std::string> keys();

    std::string getValue(const std::string &key);

    void removeKey(const std::string &key);

    void clearTable();

private:
    sqlite3* db_;    ///< Database connection
    std::string tablename_;
};

#endif //MQTT_DEMO_SQLITECLIENTB_H
