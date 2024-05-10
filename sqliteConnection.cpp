#include "sqliteConnection.h"
#include "log.h"
using namespace std;
SQLiteConnection::SQLiteConnection() : db(nullptr) {}

SQLiteConnection::~SQLiteConnection() {
    close();
}

SQLiteConnection& SQLiteConnection::getInstance() {
    static SQLiteConnection instance;
    return instance;
}

bool SQLiteConnection::open(const std::string& dbPath) {
    if (db) {
        return true; // 已经打开连接
    }
    cout << "open db file " << endl;
    //string path = "C:/Users/123/Desktop/console_socket/console_socket/feixing.db";
    int result = sqlite3_open(dbPath.c_str(), &db);
    return (result == SQLITE_OK);
}

void SQLiteConnection::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool SQLiteConnection::executeQuery(const std::string& query) {
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result != SQLITE_OK) {
        // 处理错误信息
        return false;
    }
    
    return true;
}

bool SQLiteConnection::executeNonQuery(const std::string& query) {
    //TRACE("begin to executeNonQuery");
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errorMsg);
    //TRACE("end executeNonQuery");
    if (result != SQLITE_OK) {
        // 处理错误信息
        //TRACE("Error executing query: " + std::string(errorMsg));
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

bool SQLiteConnection::executeScalar(const std::string& query, std::string& result) {
    //TRACE("begin to executeScalar");
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (prepareResult != SQLITE_OK) {
        // 处理错误信息
        //TRACE("Error executing eScalar: " + to_string(prepareResult));
        return false;
    }
    
    int stepResult = sqlite3_step(stmt);
    //TRACE("Error executing eScalar: " + to_string(stepResult));
    if (stepResult == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    } else if (stepResult == SQLITE_DONE) {
        // 没有结果
        result = "";
    } else {
        // 处理错误信息
        sqlite3_finalize(stmt);
        return false;
    }
    //TRACE("end executeScalar");
    sqlite3_finalize(stmt);
    return true;
}


bool SQLiteConnection::executeQueryWithPlaceholder(const std::string& query, const std::vector<std::string>& placeholders, std::vector<std::vector<std::string>>& results) {
    //LOG(ERROR) << " ==== executeQueryWithPlaceholder in sqlite connection ==== " ;
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (prepareResult != SQLITE_OK) {
        // 处理错误信息
        LOG(ERROR) << "Error preparing statement: " << prepareResult;
        return false;
    }
    
    for (size_t i = 0; i < placeholders.size(); ++i) {
        int bindResult = sqlite3_bind_text(stmt, i + 1, placeholders[i].c_str(), -1, SQLITE_TRANSIENT);
        if (bindResult != SQLITE_OK) {
            LOG(ERROR) << "Error binding parametert: " << bindResult;
            sqlite3_finalize(stmt);
            return false;
        }
    }
    //LOG(ERROR) << "query db get result ";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> rowResults;
        int columnCount = sqlite3_column_count(stmt);
        for (int i = 0; i < columnCount; ++i) {
            const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            rowResults.push_back(value ? value : "");
        }
        results.push_back(rowResults);
    }
    
    sqlite3_finalize(stmt);
    return true;
}



bool SQLiteConnection::executeDroneInsertQuery(const std::string& query, const std::vector<std::string>& placeholders) {
    sqlite3_stmt* stmt = nullptr;
    int prepareResult = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    
    if (prepareResult != SQLITE_OK) {
        LOG(ERROR) << "prepareResult not ok.";
        return false;
    }
    
    for (size_t i = 0; i < placeholders.size(); ++i) {
        int bindResult = sqlite3_bind_text(stmt, i + 1, placeholders[i].c_str(), -1, SQLITE_STATIC);
        if (bindResult != SQLITE_OK) {
            LOG(ERROR) << "bindResult not ok.";
            sqlite3_finalize(stmt);
            return false;
        }
    }
    
    int stepResult = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    LOG(ERROR) <<"stepResult : " << stepResult;
    if (stepResult == SQLITE_DONE) {
        LOG(ERROR) << " query executed successfully.";
        return true;
    } else {
        LOG(ERROR) << " query executed fail.";
        return false;
    }
}
