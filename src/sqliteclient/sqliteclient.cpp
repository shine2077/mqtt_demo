//
// Created by sun on 22-12-11.
//

#include "sqliteclient.h"

void SQLiteClient::open(const std::string &filename, const int mode) {
    mDb = std::make_unique<SQLite::Database>(filename, mode);
}

void SQLiteClient::createTable(const std::string &tablename, bool clean) {
    if(mDb->tableExists(tablename) ){
        if(!clean){
            return;
        }else {
            mDb->exec("DROP TABLE IF EXISTS " + tablename);
            mDb->exec("CREATE TABLE " + tablename +" (id INTEGER PRIMARY KEY, key TEXT, value TEXT)");
        }
    }else{
        mDb->exec("CREATE TABLE " + tablename +" (id INTEGER PRIMARY KEY, key TEXT, value TEXT)");
    }
}



