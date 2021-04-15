#include "Database.h"

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
    /* Open database */
    rc = sqlite3_open("test.db", &db);

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

int callback_(void* NotUsed, int argc, char** argv, char** azColName)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        DBG("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    DBG("\n");
    return 0;
}


void Database::Init()
{
    bool is_open = Open();
    if(is_open)
    {
        char* query = (char*)"SELECT* FROM(SELECT rowid, *FROM data ORDER BY rowid DESC LIMIT 3) ORDER BY rowid ASC";
        ExecuteQuery(query, &callback_);
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