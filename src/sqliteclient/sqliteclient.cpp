//
// Created by sun on 22-12-14.
//

#include "sqliteclient.h"
#include "sql.h"
#include <cassert>

SQLiteClient::~SQLiteClient() {
    int status = sqlite3_close(db_);
    ErrorCheck(status);
}

void SQLiteClient::open(const std::string &filename) {
    assert(db_ == nullptr);

    int status;
    char* err_msg = nullptr;

    status = sqlite3_open(filename.c_str(), &db_);
    if (status) {
        std::fprintf(stderr, "open error: %s\n", sqlite3_errmsg(db_));
        std::exit(1);
    }

    // Change SQLite cache size
    char cache_size[100];
    std::snprintf(cache_size, sizeof(cache_size), "PRAGMA cache_size = %d",
                  FLAGS_num_pages);
    status = sqlite3_exec(db_, cache_size, nullptr, nullptr, &err_msg);
    ExecErrorCheck(status, err_msg);

    // FLAGS_page_size is defaulted to 1024
    if (FLAGS_page_size != 1024) {
        char page_size[100];
        std::snprintf(page_size, sizeof(page_size), "PRAGMA page_size = %d",
                      FLAGS_page_size);
        status = sqlite3_exec(db_, page_size, nullptr, nullptr, &err_msg);
        ExecErrorCheck(status, err_msg);
    }
    // Change journal mode to WAL if WAL enabled flag is on
    if (FLAGS_WAL_enabled) {
        std::string WAL_stmt = "PRAGMA journal_mode = WAL";

        // LevelDB's default cache size is a combined 4 MB
        std::string WAL_checkpoint = "PRAGMA wal_autocheckpoint = 4096";
        status = sqlite3_exec(db_, WAL_stmt.c_str(), nullptr, nullptr, &err_msg);
        ExecErrorCheck(status, err_msg);
        status =
                sqlite3_exec(db_, WAL_checkpoint.c_str(), nullptr, nullptr, &err_msg);
        ExecErrorCheck(status, err_msg);
    }
}

void SQLiteClient::createTable(const std::string &tablename, bool clean) {
    tablename_ = tablename;
    int status;
    char* err_msg = nullptr;
    // Change locking mode to exclusive and create tables/index for database
    //std::string locking_stmt = "PRAGMA locking_mode = EXCLUSIVE";
    std::string create_stmt =
            "CREATE TABLE IF NOT EXISTS " + tablename_ + " (key TEXT PRIMARY KEY, value TEXT)";

    if (!FLAGS_use_rowids) create_stmt += " WITHOUT ROWID";

    std::vector< std::string > stmt_array;

    if(clean){
        std::string clean_stmt = "DROP TABLE IF EXISTS " + tablename_;
        stmt_array = {clean_stmt, create_stmt};
    }else {
        stmt_array = {create_stmt};
    }

    auto stmt_array_length = stmt_array.size();
    for (int i = 0; i < stmt_array_length; i++) {
        const char *sql = stmt_array[i].c_str();
        status = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
        ExecErrorCheck(status, err_msg);
    }
}

void SQLiteClient::instert_date(const std::string &key, const std::string &value) {
    sql::InsertModel i;
    i.insert("key", key)
            ("value", value)
            .into(tablename_);

    std::string instert_sql = i.str();

    int status;
    char* err_msg = nullptr;

    const char* sql = instert_sql.c_str();

    status = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);

    ExecErrorCheck(status, err_msg);
}

bool SQLiteClient::contains_key(const std::string &key) {
    //std::string sql = "SELECT * FROM test WHERE key=\"" + key + "\"";
    sql::SelectModel s;
    s.select("*")
            .from(tablename_)
            .where(sql::column("key") == key);
    std::string select_sql = s.str();

    int status;
    char* err_masg = nullptr;

    const char* sql = select_sql.c_str();
    status = sqlite3_exec(db_, sql, nullptr, nullptr, &err_masg);
    ExecErrorCheck(status, err_masg);

    return true;
}

std::vector<std::string> SQLiteClient::keys() {
    //std::string sql = "SELECT * FROM "+ table_;
    sql::SelectModel s;
    s.select("key")
            .from(tablename_);
    std::string select_sql = s.str();
    const char* select_sql_cstr = select_sql.c_str();

    sqlite3_stmt* stmt = nullptr;
    std::string key;
    std::vector<std::string> keys{};
    bool done = false;
    int status;
    sqlite3_prepare_v2(db_, select_sql_cstr, -1, &stmt, NULL);
    while (!done) {
        status = sqlite3_step(stmt);
        switch (status) {
            case SQLITE_ROW:
                key  = std::string( reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)) );
                keys.push_back(key);
                break;
            case SQLITE_DONE:
                done = true;
                break;
            default:
                StepErrorCheck(status);
                break;
        }
    }
    sqlite3_finalize(stmt);

    return keys;
}

std::string SQLiteClient::getValue(const std::string &key) {
    sql::SelectModel s;
    s.select("value")
    .from(tablename_)
    .where(sql::column("key") == key);

    std::string select_sql = s.str();
    const char* select_sql_cstr = select_sql.c_str();

    sqlite3_stmt*  stmt = nullptr;
    std::string value;

    bool done = false;
    int status;
    sqlite3_prepare(db_, select_sql_cstr, -1, &stmt, NULL);
    while (!done) {
        status = sqlite3_step(stmt);
        switch (status) {
            case SQLITE_ROW:
                value  = std::string( reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)) );
                break;
            case SQLITE_DONE:
                done = true;
                break;
            default:
                StepErrorCheck(status);
                break;
        }
    }
    sqlite3_finalize(stmt);

    return value;
}

void SQLiteClient::removeKey(const std::string &key) {
    sql::DeleteModel d;
    d._delete()
    .from(tablename_)
    .where(sql::column("key") == key);

    int status;
    char* err_masg = nullptr;

    const char* sql = d.str().c_str();
    status = sqlite3_exec(db_, sql, nullptr, nullptr, &err_masg);
    ExecErrorCheck(status, err_masg);
}

void SQLiteClient::clearTable() {
    sql::DeleteModel d;
    d._delete()
    .from(tablename_);

    int status;
    char* err_masg = nullptr;

    status = sqlite3_exec(db_, d.str().c_str(), nullptr, nullptr, &err_masg);
    ExecErrorCheck(status, err_masg);
}
