#include "pch.hpp"

constexpr const char* db_name = "meas_data.db";

DatabaseLogic::DatabaseLogic()
{
    m_db = std::make_unique<Sqlite3Database>();
}

void DatabaseLogic::DoGenerateGraphs()
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    DBStream db_stream(db_name, m_db);
    if(!db_stream)
    {
        LOG(LogLevel::Critical, "Failed to open the database for measurements!");
        return;
    }
    for(int i = 0; i != std::size(Sensors::Get()->last_day); i++)
    {
        Sensors::Get()->last_day[i].clear();
        Sensors::Get()->last_week[i].clear();
    }
    db_stream.SendQueryAndFetch(std::format("SELECT* FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, strftime('%H:%M:%S', time, 'unixepoch') as date_time FROM data ORDER BY rowid DESC LIMIT {}) ORDER BY rowid ASC", MAX_MEAS_QUEUE),
        std::bind(&DatabaseLogic::Query_Latest, this, std::placeholders::_1, std::placeholders::_2), 0);
    
    db_stream.SendQueryAndFetch(std::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[0]);
    db_stream.SendQueryAndFetch(std::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%d. %H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[0]);

    db_stream.SendQueryAndFetch(std::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(pm25), MAX(pm10), MAX(lux), MAX(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[1]);
    db_stream.SendQueryAndFetch(std::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(pm25), MAX(pm10), MAX(lux), MAX(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[1]);

    db_stream.SendQueryAndFetch(std::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(pm25), MIN(pm10), MIN(lux), MIN(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[2]);
    db_stream.SendQueryAndFetch(std::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(pm25), MIN(pm10), MIN(lux), MIN(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[2]);
        
    Sensors::Get()->WriteGraphs();
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#ifdef _WIN32  /* TODO: fix it */
    LOG(LogLevel::Notification, "Executing 7 query took {:.6f} ms", (double)dif / 1000000.0);
#endif
    last_db_update = std::chrono::steady_clock::now();
}

void DatabaseLogic::GenerateGraphs()
{
    if(m_graph_future.valid())
        if(m_graph_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            return;
    m_graph_future = std::async(&DatabaseLogic::DoGenerateGraphs, this);
}

void DatabaseLogic::InsertMeasurement(std::unique_ptr<Measurement> &m)
{
    if(m_graph_future.valid())  /* wait for generating to complete to avoid data races */
        m_graph_future.get();

    DBStream db_stream(db_name, m_db);
    if(!db_stream)
    {
        LOG(LogLevel::Critical, "Failed to open the database for measurements!");
        return;
    }

    char query[512] = "CREATE TABLE IF NOT EXISTS data(\
sensor_id INT NOT NULL,\
temp            INT     NOT NULL,\
hum            INT     NOT NULL,\
co2            INT     NOT NULL,\
voc            INT     NOT NULL,\
pm25            INT     NOT NULL,\
pm10            INT     NOT NULL,\
lux            INT     NOT NULL,\
cct            INT     NOT NULL,\
time            INT     NOT NULL);";
    db_stream.ExecuteQuery(query);
    snprintf(query, sizeof(query), "INSERT INTO data(sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, time) VALUES(1, %d, %d, %d, %d, %d, %d, %d, %d, strftime('%%s', 'now', 'localtime'))",
        static_cast<int>(std::round(m->temp * 10.f)), static_cast<int>(std::round(m->hum * 10.f)), m->co2, m->voc, m->pm25, m->pm10, m->lux, m->cct);
    db_stream.ExecuteQuery(query);
}

void DatabaseLogic::Query_Latest(std::unique_ptr<Result>& result, std::any param)
{
    int cols = result->GetColumnCount();
    while(1)
    {
        bool is_ok = result->StepNext();
        if(!is_ok)
            break;

        float temp = boost::lexical_cast<float>(result->GetColumnText(2)) / 10.f;
        float hum = boost::lexical_cast<float>(result->GetColumnText(3)) / 10.f;
        int co2 = result->GetColumnInt(4);
        int voc = result->GetColumnInt(5);
        int pm25 = result->GetColumnInt(6);
        int pm10 = result->GetColumnInt(7);
        int lux = result->GetColumnInt(8);
        int cct = result->GetColumnInt(9);
        std::string time{ reinterpret_cast<const char*>(result->GetColumnText(10)) };
        Sensors::Get()->AddMeasurement(std::make_unique<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time)));
    }
}

void DatabaseLogic::Query_MeasFromPast(std::unique_ptr<Result>& result, std::any param)
{
    int cols = result->GetColumnCount();
    std::vector<std::unique_ptr<Measurement>>* vector_ptr = std::any_cast<decltype(vector_ptr)>(param);
    while(1)
    {
        bool is_ok = result->StepNext();
        if(!is_ok)
            break;

        float temp = boost::lexical_cast<float>(result->GetColumnText(0)) / 10.f;
        float hum = boost::lexical_cast<float>(result->GetColumnText(1)) / 10.f;
        int co2 = result->GetColumnInt(2);
        int voc = result->GetColumnInt(3);
        int pm25 = result->GetColumnInt(4);
        int pm10 = result->GetColumnInt(5);
        int lux = result->GetColumnInt(6);
        int cct = result->GetColumnInt(7);
        std::string time{ reinterpret_cast<const char*>(result->GetColumnText(8)) };
        vector_ptr->push_back(std::make_unique<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time)));
    }
}