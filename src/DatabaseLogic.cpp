#include "pch.hpp"

constexpr const char* db_name = "meas_data.db";

DatabaseLogic::DatabaseLogic()
{
    m_db = std::make_unique<Sqlite3Database>();
}

DatabaseLogic::~DatabaseLogic()
{
    m_destructing = true;
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
    db_stream.SendQueryAndFetch(std::format("SELECT * FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, co, pm25, pm10, pressure, r, g, b, lux, cct, uv, strftime('%H:%M:%S', time, 'unixepoch') as date_time FROM data ORDER BY rowid DESC LIMIT {}) ORDER BY rowid ASC", MAX_MEAS_QUEUE),
        std::bind(&DatabaseLogic::Query_Latest, this, std::placeholders::_1, std::placeholders::_2), 0);
    
    db_stream.SendQueryAndFetch(std::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(co), AVG(pm25), AVG(pm10), AVG(pressure), AVG(r), AVG(g), AVG(b), AVG(lux), AVG(cct), AVG(uv), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[0]);
    db_stream.SendQueryAndFetch(std::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(co), AVG(pm25), AVG(pm10), AVG(pressure), AVG(r), AVG(g), AVG(b), AVG(lux), AVG(cct), AVG(uv), strftime('%Y-%m-%d %H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[0]);

    db_stream.SendQueryAndFetch(std::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(co), MAX(pm25), MAX(pm10), MAX(pressure), MAX(r), MAX(g), MAX(b), MAX(lux), MAX(cct), MAX(uv), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[1]);
    db_stream.SendQueryAndFetch(std::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(co), MAX(pm25), MAX(pm10), MAX(pressure), MAX(r), MAX(g), MAX(b), MAX(lux), MAX(cct), MAX(uv), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[1]);

    db_stream.SendQueryAndFetch(std::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(co), MIN(pm25), MIN(pm10), MIN(pressure), MIN(r), MIN(g), MIN(b), MIN(lux), MIN(cct), MIN(uv), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_1 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_day[2]);
    db_stream.SendQueryAndFetch(std::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(co), MIN(pm25), MIN(pm10), MIN(pressure), MIN(r), MIN(g), MIN(b), MIN(lux), MIN(cct), MIN(uv), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, m_graphs_hours_2 * 3600),
        std::bind(&DatabaseLogic::Query_MeasFromPast, this, std::placeholders::_1, std::placeholders::_2), &Sensors::Get()->last_week[2]);
        
    Sensors::Get()->WriteGraphs();
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    LOG(LogLevel::Notification, "Executing 7 query took {:.6f} ms", (double)dif / 1000000.0);
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

    char query[] = "CREATE TABLE IF NOT EXISTS data(\
sensor_id INT NOT NULL,\
temp            INT     NOT NULL,\
hum            INT     NOT NULL,\
co2            INT     NOT NULL,\
voc            INT     NOT NULL,\
co            INT     NOT NULL,\
pm25            INT     NOT NULL,\
pm10            INT     NOT NULL,\
pressure            INT     NOT NULL,\
r            INT     NOT NULL,\
g            INT     NOT NULL,\
b            INT     NOT NULL,\
lux            INT     NOT NULL,\
cct            INT     NOT NULL,\
uv            INT     NOT NULL,\
time            INT     NOT NULL);";
    db_stream.ExecuteQuery(query);
    snprintf(query, sizeof(query), "INSERT INTO data(sensor_id, temp, hum, co2, voc, co, pm25, pm10, pressure, r, g, b, lux, cct, uv, time) VALUES(1, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, strftime('%%s', 'now', 'localtime'))",
        static_cast<int>(std::round(m->temp * 10.f)), static_cast<int>(std::round(m->hum * 10.f)), m->co2, m->voc, m->co, m->pm25, m->pm10, 
        static_cast<int>(std::round(m->pressure * 10.f)), static_cast<int>(std::round(m->r * 10.f)), static_cast<int>(std::round(m->g * 10.f)), static_cast<int>(std::round(m->b * 10.f)), 
        m->lux, m->cct, m->uv);
    db_stream.ExecuteQuery(query);
}

void DatabaseLogic::Query_Latest(std::unique_ptr<Result>& result, std::any param)
{
    int cols = result->GetColumnCount();
    while(!m_destructing)
    {
        bool is_ok = result->StepNext();
        if(!is_ok)
            break;

        float temp = boost::lexical_cast<float>(result->GetColumnText(2)) / 10.f;
        float hum = boost::lexical_cast<float>(result->GetColumnText(3)) / 10.f;
        int co2 = result->GetColumnInt(4);
        int voc = result->GetColumnInt(5);
        int co = result->GetColumnInt(6);
        int pm25 = result->GetColumnInt(7);
        int pm10 = result->GetColumnInt(8);
        
        float pressure = 0.0f;
        if(result->GetColumnText(9))
            pressure = boost::lexical_cast<float>(result->GetColumnText(9)) / 10.f;

        float r = 0;
        if(result->GetColumnText(10))
            r = boost::lexical_cast<float>(result->GetColumnText(10)) / 10.f;

        float g = 0;
        if(result->GetColumnText(11))
            g = boost::lexical_cast<float>(result->GetColumnText(11)) / 10.f;

        float b = 0;
        if(result->GetColumnText(12))
            b = boost::lexical_cast<float>(result->GetColumnText(12)) / 10.f;

        int lux = result->GetColumnInt(13);
        int cct = result->GetColumnInt(14);
        int uv = result->GetColumnInt(15);
        std::string time{ reinterpret_cast<const char*>(result->GetColumnText(16)) };
        if(!m_destructing)
            Sensors::Get()->AddMeasurement(std::make_unique<Measurement>(temp, hum, co2, voc, co, pm25, pm10, pressure, r, g, b, lux, cct, uv, std::move(time)));
    }
}

void DatabaseLogic::Query_MeasFromPast(std::unique_ptr<Result>& result, std::any param)
{
    int cols = result->GetColumnCount();
    std::vector<std::unique_ptr<Measurement>>* vector_ptr = std::any_cast<decltype(vector_ptr)>(param);
    while(!m_destructing)
    {
        bool is_ok = result->StepNext();
        if(!is_ok)
            break;

        float temp = boost::lexical_cast<float>(result->GetColumnText(0)) / 10.f;
        float hum = boost::lexical_cast<float>(result->GetColumnText(1)) / 10.f;
        int co2 = result->GetColumnInt(2);
        int voc = result->GetColumnInt(3);
        int co = result->GetColumnInt(4);
        int pm25 = result->GetColumnInt(5);
        int pm10 = result->GetColumnInt(6);
        float pressure = 0.0f;
        if(result->GetColumnText(7))
            pressure = boost::lexical_cast<float>(result->GetColumnText(7)) / 10.f;

        float r = 0;
        if(result->GetColumnText(8))
            r = boost::lexical_cast<float>(result->GetColumnText(8)) / 10.f;

        float g = 0;
        if(result->GetColumnText(9))
            g = boost::lexical_cast<float>(result->GetColumnText(9)) / 10.f;

        float b = 0;
        if(result->GetColumnText(10))
            b = boost::lexical_cast<float>(result->GetColumnText(10)) / 10.f;

        int lux = result->GetColumnInt(11);
        int cct = result->GetColumnInt(12);
        int uv = result->GetColumnInt(13);
        std::string time{ reinterpret_cast<const char*>(result->GetColumnText(14)) };
        if(!m_destructing)
            vector_ptr->push_back(std::make_unique<Measurement>(temp, hum, co2, voc, co, pm25, pm10, pressure, r, g, b, lux, cct, uv, std::move(time)));
    }
}