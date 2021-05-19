#pragma once

#include "utils/CSingleton.h"

#include "Logger.h"

#include <inttypes.h>
#include <map>
#include <string>
#include <any>

#include <sqlite/sqlite3.h>

class Measurement;

class Database : public CSingleton < Database >
{
    friend class CSingleton < Database >;

public:
    Database() = default;
    void Init(void);
    void InsertMeasurement(std::shared_ptr<Measurement>& m);

private:
    void SendQuery(std::string&& query, void(Database::* execute_function)(sqlite3_stmt* stmt, std::any param), std::any params);
    

    bool ExecuteQuery(char* query, int (*callback)(void*, int, char**, char**) = NULL);
    bool Open(void);

    void Query_Latest(sqlite3_stmt* stmt, std::any params);
    void Query_MeasFromPast(sqlite3_stmt* stmt, std::any params);

    sqlite3* db;
    int rc;
    char* sql;
};