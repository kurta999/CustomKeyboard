#pragma once

#include "utils/CSingleton.hpp"

#include "Logger.hpp"
#include "IDatabase.hpp"

#include <inttypes.h>
#include <map>
#include <string>
#include <any>
#include <future>

#include <sqlite/sqlite3.h>

class Measurement;
class Result;

class DatabaseLogic : public CSingleton < DatabaseLogic >
{
    friend class CSingleton < DatabaseLogic >;

public:
    DatabaseLogic();
    ~DatabaseLogic();

    // !\brief Execute graph generation
    void DoGenerateGraphs();

    // !\brief Start async graph generation
    void GenerateGraphs();

    // !\brief Add measurements to measurement queue
    // !\param m [in] Reference to measurement's uniue pointer
    void InsertMeasurement(std::unique_ptr<Measurement>& m);

    // !\brief Set interval for graph
    // !\param id [in] ID of graph
    // !\param val [in] Interval [hour]
    void SetGraphHours(uint8_t id, uint32_t val)
    {
        if(!id)
            m_graphs_hours_1 = val;
        else
            m_graphs_hours_2 = val;
    }

    // !\brief Get interval for graph
    // !\param id [in] ID of graph
    // !\return Interval [hour]
    uint32_t GetGraphHours(uint8_t id)
    {
        if(!id)
            return m_graphs_hours_1;
       return m_graphs_hours_2;
    }

    // !\brief Time for storing last database update time
    std::chrono::steady_clock::time_point last_db_update;
private:
    // !\brief Function for processing the results of query for latest measurements
    // !\param result [in] Reference to result's uniue pointer
    // !\param param [in] Additional parameters
    void Query_Latest(std::unique_ptr<Result>& result, std::any param);

    // !\brief Function for processing the results of qeury for measurements from the past
    // !\param result [in] Reference to result's uniue pointer
    // !\param param [in] Additional parameters
    void Query_MeasFromPast(std::unique_ptr<Result>& result, std::any param);

    // !\brief Pointer to database
    std::unique_ptr<IDatabase> m_db{nullptr};

    // !\brief Future for executing async database read for graph generation
    std::future<void> m_graph_future;

    // !\brief Interval for graph 1
    uint32_t m_graphs_hours_1 = 24;  /* One day */

    // !\brief Interval for graph 2
    uint32_t m_graphs_hours_2 = 168;  /* One week */

    // !\brief Is destruction in progess?
    bool m_destructing = false;
};