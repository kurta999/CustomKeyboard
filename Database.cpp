#include "Database.h"
#include "Sensors.h"
#include "boost/lexical_cast.hpp"

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


void Database::Init()
{
    bool is_open = Open();
    if(!is_open)
        return;
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db, (char*)"SELECT* FROM(SELECT rowid, sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, strftime('%H:%M:%S', time, 'unixepoch') as date_time \
FROM data ORDER BY rowid DESC LIMIT 30) ORDER BY rowid ASC", -1, &stmt, 0) == SQLITE_OK)
    {
        int cols = sqlite3_column_count(stmt);
        int result = 0;

        while(true)
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

        Sensors::Get()->WriteGraphs();
        sqlite3_finalize(stmt);
    }
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