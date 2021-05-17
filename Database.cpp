#include "Database.h"
#include "Sensors.h"
#include "boost/lexical_cast.hpp"

int constexpr one_week_seconds = 60 * 60 * 24 * 7;

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
    LOGMSG(normal, "Query executed successfully");
    return true;
}

bool Database::Open(void)
{
    rc = sqlite3_open("test.db", &db);  /* Open database */
    if(rc)
    {
        LOGMSG(error, "Can't open database: {}", sqlite3_errmsg(db));
    }
    else
    {
        LOGMSG(normal, "Opened database successfully");
    }
    return rc == 0;
}

// use function pointers for queryes
/* last 1 hour:
* SELECT* FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, strftime('%H:%M:%S', time, 'unixepoch') as date_time, time FROM data WHERE time > (strftime('%s', 'now') - 3600) ORDER BY rowid DESC LIMIT 300) ORDER BY rowid ASC
*/



void Database::SendQuery(std::string&& query, void(Database::*execute_function)(sqlite3_stmt* stmt))
{
    sqlite3_stmt* stmt;
    int ret = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if(ret == SQLITE_OK)
    {
        (this->*execute_function)(stmt);
        sqlite3_finalize(stmt);
    }
}

/*
* SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct) FROM (SELECT *, NTILE(50) OVER (ORDER BY time) grp FROM data) GROUP BY grp

last X times average everything:
SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE(5) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- 3600)) GROUP BY grp

SELECT MAX(temp), MAX(hum), MAX(co2), MAX(voc), MAX(pm25), MAX(pm10), MAX(lux), MAX(cct), strftime('%H:%M:%S', time, 'unixepoch') FROM (SELECT *, NTILE(5) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- 3600)) GROUP BY grp
*/
void Database::Init()
{
    bool is_open = Open();
    if(!is_open)
        return;
    
    // TODO: fix time intervals
    SendQuery(fmt::format("SELECT* FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, strftime('%H:%M:%S', time, 'unixepoch') as date_time \
FROM data ORDER BY rowid DESC LIMIT {}) ORDER BY rowid ASC", MAX_MEAS_QUEUE), &Database::Query_Latest);
    SendQuery(fmt::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%H:%M:%S', time, 'unixepoch') \
FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- 360000)) GROUP BY grp", MAX_MEAS_QUEUE), &Database::Query_1Day);
    SendQuery(fmt::format("SELECT AVG(temp), AVG(hum), AVG(co2), AVG(voc), AVG(pm25), AVG(pm10), AVG(lux), AVG(cct), strftime('%H:%M:%S', time, 'unixepoch') \
FROM (SELECT *, NTILE({}) OVER (ORDER BY time) grp FROM data WHERE time > (strftime('%s', 'now')- {})) GROUP BY grp", MAX_MEAS_QUEUE, one_week_seconds), &Database::Query_1Week);
}

void Database::InsertMeasurement(std::shared_ptr<Measurement> &m)
{
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
        snprintf(query, sizeof(query), "INSERT INTO data(sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, time) VALUES(1, %d, %d, %d, %d, %d, %d, %d, %d, strftime('%%s', 'now'))",
            (int)roundf(m->temp * 10.f), (int)roundf(m->hum * 10.f), m->co2, m->voc, m->pm25, m->pm10, m->lux, m->cct);
        ExecuteQuery(query);
        sqlite3_close(db);
    }
}


void Database::Query_Latest(sqlite3_stmt* stmt)
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
        std::string time = std::string((const char*)sqlite3_column_text(stmt, 10));

        std::shared_ptr<Measurement> m = std::make_shared<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time));
        Sensors::Get()->AddMeasurement(m);
    }
}

void Database::Query_1Day(sqlite3_stmt* stmt)
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

        float temp = boost::lexical_cast<float>(sqlite3_column_text(stmt, 0)) / 10.f;
        float hum = boost::lexical_cast<float>(sqlite3_column_text(stmt, 1)) / 10.f;
        int co2 = sqlite3_column_int(stmt, 2);
        int voc = sqlite3_column_int(stmt, 3);
        int pm25 = sqlite3_column_int(stmt, 4);
        int pm10 = sqlite3_column_int(stmt, 5);
        int lux = sqlite3_column_int(stmt, 6);
        int cct = sqlite3_column_int(stmt, 7);
        std::string time = std::string((const char*)sqlite3_column_text(stmt, 8));

        std::shared_ptr<Measurement> m = std::make_shared<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time));
        Sensors::Get()->AddMeasurementDay(m);
    }
    Sensors::Get()->WriteGraphs();
}

void Database::Query_1Week(sqlite3_stmt* stmt)
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

        float temp = boost::lexical_cast<float>(sqlite3_column_text(stmt, 0)) / 10.f;
        float hum = boost::lexical_cast<float>(sqlite3_column_text(stmt, 1)) / 10.f;
        int co2 = sqlite3_column_int(stmt, 2);
        int voc = sqlite3_column_int(stmt, 3);
        int pm25 = sqlite3_column_int(stmt, 4);
        int pm10 = sqlite3_column_int(stmt, 5);
        int lux = sqlite3_column_int(stmt, 6);
        int cct = sqlite3_column_int(stmt, 7);
        std::string time = std::string((const char*)sqlite3_column_text(stmt, 8));

        std::shared_ptr<Measurement> m = std::make_shared<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(time));
        Sensors::Get()->AddMeasurementWeek(m);
    }
    Sensors::Get()->WriteGraphs();
}