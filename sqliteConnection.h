#ifndef _SQLITE_CONNECTOR_H_
#define _SQLITE_CONNECTOR_H_

#include <iostream>
#include <sqlite3.h>
#include <string>
#include "log1.h"

class SQLiteConnection {
public:
    static SQLiteConnection& getInstance();
    
    bool open(const std::string& dbPath);
    void close();
    bool executeQuery(const std::string& query);
    bool executeNonQuery(const std::string& query);
    bool executeScalar(const std::string& query, std::string& result);
    bool executeQueryWithPlaceholder(const std::string& query, const std::vector<std::string>& placeholders, std::vector<std::vector<std::string>>& results);
    bool executeDroneInsertQuery(const std::string& query, const std::vector<std::string>& placeholders);
private:
    SQLiteConnection();
    ~SQLiteConnection();
    SQLiteConnection(const SQLiteConnection&) = delete;
    SQLiteConnection& operator=(const SQLiteConnection&) = delete;
    
    sqlite3* db;
};
#endif