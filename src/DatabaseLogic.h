#pragma once

#include "utils/CSingleton.h"

#include "Logger.h"

#include <inttypes.h>
#include <map>
#include <string>
#include <any>
#include <future>

#include <sqlite/sqlite3.h>

class Measurement;
class Result;
class IDatabase;

class DatabaseLogic : public CSingleton < DatabaseLogic >
{
    friend class CSingleton < DatabaseLogic >;

public:
    DatabaseLogic();
    ~DatabaseLogic() = default;

    void DoGenerateGraphs();
    void GenerateGraphs();
    void InsertMeasurement(std::unique_ptr<Measurement>& m);
    void SetGraphHours(uint8_t id, uint32_t val)
    {
        if(!id)
            m_graphs_hours_1 = val;
        else
            m_graphs_hours_2 = val;
    }
    uint32_t GetGraphHours(uint8_t id)
    {
        if(!id)
            return m_graphs_hours_1;
       return m_graphs_hours_2;
    }

    time_t last_db_update;

private:
    void Query_Latest(std::unique_ptr<Result>& result, std::any param);
    void Query_MeasFromPast(std::unique_ptr<Result>& result, std::any param);

    std::unique_ptr<IDatabase> m_db{nullptr};
    std::future<void> m_graph_future;
    uint32_t m_graphs_hours_1 = 24;  /* One day */
    uint32_t m_graphs_hours_2 = 168;  /* One week */
};