#pragma once

#include <string>

class Result;
class IDatabase
{
public:
    virtual ~IDatabase() { }

    virtual bool Open(const char* db_name) = 0;
    virtual bool Close() = 0;
    virtual void ExecuteQuery(std::string&& query) = 0;
    virtual void SendQueryAndFetch(std::string&& query, std::function<void(std::unique_ptr<Result>&, std::any)> function, std::any params) = 0;
};
