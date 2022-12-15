//
// Created by sun on 22-12-11.
//
#include "../src/sqliteclient/sqliteclient.h"

int main()
{
    SQLiteClient* sqlite_client = new SQLiteClient();
    sqlite_client->open("./test", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
    sqlite_client->createTable("test", true);
//    sqlite_client->instert_date("c1","hello");
//    sqlite_client->instert_date("c2","sqlite3");
//    sqlite_client->instert_date("c3","hi");
//    bool result = sqlite_client->contains_key("c1");
//    std::cout<<result<<std::endl;
//    result = sqlite_client->contains_key("c2");
//    std::cout<<result<<std::endl;
//    result = sqlite_client->contains_key("c3");
//    std::cout<<result<<std::endl;
//    std::vector<std::string> keys = sqlite_client->keys();
//    std::cout<<sqlite_client->getValue("c1")<<std::endl;
//
//    sqlite_client->removeKey("c3");
    sqlite_client->exec("insert into test(key, value) values('c1', '\003\000\000\000\001\000\000\000data/rand\000\030\000\000\000\061,2022-12-14 21:16:11,33\001\000\000\000\001\000\000\000')");

    delete sqlite_client;
    return 0;
}
