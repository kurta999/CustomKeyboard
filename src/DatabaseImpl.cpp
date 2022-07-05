#include "pch.hpp"

Result::Result(sqlite3_stmt* stmt) :
    m_stmt(stmt)
{

}

Result::~Result()
{
    if(m_stmt)
        sqlite3_finalize(m_stmt);
}

int Result::GetColumnInt(int col)
{
    int ret = sqlite3_column_int(m_stmt, col);
    return ret;
}

const uint8_t* Result::GetColumnText(int col)
{
    const uint8_t* ret = sqlite3_column_text(m_stmt, col);
    return ret;
}

int Result::GetColumnCount()
{
    int cols = sqlite3_column_count(m_stmt);
    return cols;
}

bool Result::StepNext()
{
    int err_code = sqlite3_step(m_stmt);
    if(err_code == SQLITE_DONE)
        return false;
    if(err_code != SQLITE_ROW)
    {
        return false;
        //DBG("error: %s!\n", sqlite3_errmsg(db));
    }
    return true;
}

bool Sqlite3Database::Open(const char* db_name)
{
    int ret = sqlite3_open(db_name, &db);
    if(ret != SQLITE_OK)
        LOG(LogLevel::Error, "Can't open database: {}", sqlite3_errmsg(db));
    return ret == SQLITE_OK;
}

bool Sqlite3Database::Close()
{
    int ret = sqlite3_close(db);
    if(ret != SQLITE_OK)
        LOG(LogLevel::Error, "Can't close database: {}", sqlite3_errmsg(db));
    return ret == SQLITE_OK;
}

void Sqlite3Database::ExecuteQuery(std::string&& query)
{
    char* zErrMsg = 0;
    int ret = sqlite3_exec(db, query.c_str(), NULL, 0, &zErrMsg);
    if(ret != SQLITE_OK)
    {
        LOG(LogLevel::Error, "SQL Error: {}", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void Sqlite3Database::SendQueryAndFetch(std::string&& query, std::function<void(std::unique_ptr<Result>&, std::any)> function, std::any params)
{
    sqlite3_stmt* stmt;
    int ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if(ret == SQLITE_OK)
    {
        std::unique_ptr<Result> res = std::make_unique<Result>(stmt);
        function(res, params);
    }
}

DBStream::DBStream(const char* db_name, std::unique_ptr<IDatabase>& db) :
    m_db(db)
{
    is_opened = m_db->Open(db_name);
}

DBStream::~DBStream() noexcept(false)
{
    if(is_opened)
        m_db->Close();
}

DBStream::operator bool() const
{
    return is_opened;
}

void DBStream::ExecuteQuery(std::string&& query)
{
    m_db->ExecuteQuery(std::move(query));
}

void DBStream::SendQueryAndFetch(std::string&& query, std::function<void(std::unique_ptr<Result>&, std::any)> function, std::any params)
{
    m_db->SendQueryAndFetch(std::move(query), function, params);
}
