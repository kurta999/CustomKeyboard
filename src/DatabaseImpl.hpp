#pragma once

#include <string>
#include <functional>
#include <any>

#include <sqlite/sqlite3.h>

#include "IDatabase.hpp"

class Result
{
public:
    Result(sqlite3_stmt* stmt);

    ~Result();

    int GetColumnInt(int col);
    const uint8_t* GetColumnText(int col);
    int GetColumnCount();
    bool StepNext();
private:
    sqlite3_stmt* m_stmt = nullptr;
};

class Sqlite3Database final : public IDatabase
{
public:
    Sqlite3Database() = default;
    virtual ~Sqlite3Database() { };

    bool Open(const char* db_name) override;
    bool Close() override;
    void ExecuteQuery(std::string&& query) override;
    void SendQueryAndFetch(std::string&& query, std::function<void(std::unique_ptr<Result>&, std::any)> function, std::any params) override;
private:
    sqlite3* db = nullptr;
};

class DBStream
{
public:
    DBStream(const char* db_name, std::unique_ptr<IDatabase>& db);
    ~DBStream() noexcept(false);

    operator bool() const;
    void ExecuteQuery(std::string&& query);
    void SendQueryAndFetch(std::string&& query, std::function<void(std::unique_ptr<Result>&, std::any)> function, std::any params);
private:
    std::unique_ptr<IDatabase>& m_db;
    bool is_opened = false;
};
