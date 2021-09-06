#include "pch.h"

constexpr const char* db_name = "meas_data.db";

bool Database::ExecuteQuery(char* query, int (*callback)(void*, int, char**, char**))
{
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        LOGMSG(error, "SQL Error: {}", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

bool Database::Open()
{
    rc = sqlite3_open(db_name, &db);
    if(rc != SQLITE_OK)
        LOGMSG(error, "Can't open database: {}", sqlite3_errmsg(db));
    return rc == 0;
}

void Database::SendQuery(std::string&& query, void(Database::*execute_function)(sqlite3_stmt* stmt, std::any param), std::any params)
{
    sqlite3_stmt* stmt;
    int ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if(ret == SQLITE_OK)
    {
        (this->*execute_function)(stmt, params);
        sqlite3_finalize(stmt);
    }
}

void Database::DoGenerateGraphs()
{
    bool is_open = Open();
    if(!is_open)
        return;
    
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    for(int i = 0; i != std::size(Sensors::Get()->last_day); i++)
    {
        Sensors::Get()->last_day[i].clear();
        Sensors::Get()->last_week[i].clear();
    }
    SendQuery(fmt::format("SELECT* FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, strftime('%H:%M:%S', time, 'unixepoch') as date_time FROM data ORDER BY rowid DESC LIMIT {}) ORDER BY rowid ASC", MAX_MEAS_QUEUE), 
        &Database::Query_Latest, 0);
    
    SendQuery(fmt::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_1 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_day[0]);
    SendQuery(fmt::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%d. %H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_2 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_week[0]);

    SendQuery(fmt::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(pm25), MAX(pm10), MAX(lux), MAX(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_1 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_day[1]);
    SendQuery(fmt::format("SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(pm25), MAX(pm10), MAX(lux), MAX(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_2 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_week[1]);

    SendQuery(fmt::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(pm25), MIN(pm10), MIN(lux), MIN(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_1 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_day[2]);
    SendQuery(fmt::format("SELECT MIN(temp), MIN(hum), MIN(co2), MIN(voc), MIN(pm25), MIN(pm10), MIN(lux), MIN(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, graphs_hours_2 * 3600), 
        &Database::Query_MeasFromPast, &Sensors::Get()->last_week[2]);

    Sensors::Get()->WriteGraphs();
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

    LOGMSG(notification, "Executing 7 query took {:.6f} ms", (double)dif / 1000000.0);
    sqlite3_close(db);
    time(&last_db_update);
}

void Database::GenerateGraphs()
{
    if(graph_future.valid())
        if(graph_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            return;
    graph_future = std::async(&Database::DoGenerateGraphs, this);
}

void Database::InsertMeasurement(std::unique_ptr<Measurement> &m)
{
    if(graph_future.valid())  /* wait for generating to complete to avoid data races */
        graph_future.get();

    bool is_open = Open();
    if(is_open)
    {
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
        ExecuteQuery(query);
        snprintf(query, sizeof(query), "INSERT INTO data(sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, time) VALUES(1, %d, %d, %d, %d, %d, %d, %d, %d, strftime('%%s', 'now', 'localtime'))",
            static_cast<int>(std::round(m->temp * 10.f)), static_cast<int>(std::round(m->hum * 10.f)), m->co2, m->voc, m->pm25, m->pm10, m->lux, m->cct);
        ExecuteQuery(query);
        sqlite3_close(db);
    }
}

void Database::Query_Latest(sqlite3_stmt* stmt, std::any param)
{
    int cols = sqlite3_column_count(stmt);
    int result = 0;

    while(1)
    {
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_DONE)
            break;
        if(rc != SQLITE_ROW)
        {
            DBG("error: %s!\n", sqlite3_errmsg(db));
            break;
        }

        float temp = boost::lexical_cast<float>(sqlite3_column_text(stmt, 2)) / 10.f;
        float hum = boost::lexical_cast<float>(sqlite3_column_text(stmt, 3)) / 10.f;
        int co2 = sqlite3_column_int(stmt, 4);
        int voc = sqlite3_column_int(stmt, 5);
        int pm25 = sqlite3_column_int(stmt, 6);
        int pm10 = sqlite3_column_int(stmt, 7);
        int lux = sqlite3_column_int(stmt, 8);
        int cct = sqlite3_column_int(stmt, 9);
        std::string time{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) };
        Sensors::Get()->AddMeasurement(std::make_unique<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time)));
    }
}

void Database::Query_MeasFromPast(sqlite3_stmt* stmt, std::any param)
{
    int cols = sqlite3_column_count(stmt);
    int result = 0;

    std::vector<std::unique_ptr<Measurement>>* vector_ptr = std::any_cast<decltype(vector_ptr)>(param);
    while(1)
    {
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_DONE)
            break;
        if(rc != SQLITE_ROW)
        {
            DBG("error: %s!\n", sqlite3_errmsg(db));
            break;
        }

        float temp = boost::lexical_cast<float>(sqlite3_column_text(stmt, 0)) / 10.f;
        float hum = boost::lexical_cast<float>(sqlite3_column_text(stmt, 1)) / 10.f;
        int co2 = sqlite3_column_int(stmt, 2);
        int voc = sqlite3_column_int(stmt, 3);
        int pm25 = sqlite3_column_int(stmt, 4);
        int pm10 = sqlite3_column_int(stmt, 5);
        int lux = sqlite3_column_int(stmt, 6);
        int cct = sqlite3_column_int(stmt, 7);
        std::string time{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) };
        vector_ptr->push_back(std::make_unique<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time)));
    }
}