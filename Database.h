#pragma once

#include "utils/CSingleton.h"

#include "Logger.h"

#include <inttypes.h>
#include <map>
#include <string>


#include <sqlite/sqlite3.h>

#include "Sensors.h"

class Database : public CSingleton < Database >
{
    friend class CSingleton < Database >;

public:
    Database() = default;
    void Init(void);
    void InsertMeasurement(std::shared_ptr<Measurement>& m);

private:
    bool ExecuteQuery(char* query, int (*callback)(void*, int, char**, char**) = NULL);
    bool Open(void);
    int Callback(void* NotUsed, int argc, char** argv, char** azColName);
    sqlite3* db;
    int rc;
    char* sql;
};