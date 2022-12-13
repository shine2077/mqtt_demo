//
// Created by sun on 22-12-11.
//

#include "sqliteclient.h"
#include "sql.h"

void SQLiteClient::open(const std::string &filename, const int mode) {
    mDb = std::make_unique<SQLite::Database>(filename, mode);
}

void SQLiteClient::createTable(const std::string &tablename, bool clean) {
    if(mDb->tableExists(tablename) ){
        if(!clean){
            table_ = tablename;
            return;
        }else {
            mDb->exec("DROP TABLE IF EXISTS " + tablename);
            mDb->exec("CREATE TABLE " + tablename +" (key TEXT PRIMARY KEY, value BLOB)");
        }
    }else{
        mDb->exec("CREATE TABLE " + tablename +" (key TEXT PRIMARY KEY, value BLOB)");
    }
    table_ = tablename;
}

void SQLiteClient::instert_date(const std::string &key, const std::string &content) {
    try{
        sql::InsertModel i;
        i.insert("key", key)
                ("value", content)
                .into(table_);
        mDb->exec(i.str());
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
    }
}

bool SQLiteClient::contains_key(const std::string &key) {
    try{
        std::string sql = "SELECT * FROM test WHERE key=\"" + key + "\"";
        SQLite::Statement query(*mDb.get(), sql);
        while (query.executeStep())
        {
            return true;
        }
        return false;

    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return false; // unexpected error
    }
}

std::vector<std::string> SQLiteClient::keys() {
    try
    {
        std::string sql = "SELECT * FROM "+ table_;
        SQLite::Statement query(*mDb.get(), sql);
        std::vector<std::string> mkeys = {};
        while (query.executeStep())
        {
            mkeys.push_back(query.getColumn(0));
        }
        return mkeys;

    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return {};
    }
}

std::string SQLiteClient::getValue(const std::string &key) {
    try{
        std::string sql = "SELECT * FROM "+  table_ + " WHERE key=\"" + key + "\"";
        SQLite::Statement query(*mDb.get(), sql);
        while (query.executeStep())
        {
            return std::string(query.getColumn(1).getText());
        }
        return std::string();

    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return std::string();
    }
}

void SQLiteClient::removeKey(const std::string &key) {
    try
    {
        std::string sql = "DELETE FROM "+  table_ + " WHERE key=\"" + key + "\"";
        mDb->exec(sql);
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
    }
}

void SQLiteClient::clearTable() {
    try
    {
        std::string sql = "DELETE FROM " +  table_;
        mDb->exec(sql);
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
    }
}

void SQLiteClient::exec(const std::string &sql) {
    try
    {
        mDb->exec(sql);
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
    }

}

std::string SQLiteClient::formatSQL(const char *format, ...) {
    char buf[4096];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    return buf;
}



