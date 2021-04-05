#include <boost/asio.hpp>
#include "Sensors.h"
#include "Logger.h"

#include "sqlite/sqlite3.h"
#include <iostream>
#include <memory>
#include <utility>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
using namespace std::chrono_literals;
const int max_length = 1024;
namespace po = boost::program_options;

sqlite3* db;

int rc;
char* sql;

bool DB_ExecuteQuery(char* query, int (*callback)(void*, int, char**, char**) = NULL)
{
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        //Logger::Get()->Log(error, "SQL Error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    //Logger::Get()->Log(normal, "Query executed successfully");
    return true;
}

bool DB_Open(void)
{
    /* Open database */
    rc = sqlite3_open("test.db", &db);

    if (rc)
    {
        //Logger::Get()->Log(severity_level::error, "Can't open database: %s", sqlite3_errmsg(db));
    }
    else
    {
        //Logger::Get()->Log(normal, "Opened database successfully");
    }
    return rc == 0;
}

void session(tcp::socket sock)
{
    try
    {
        for (;;)
        {
            char data[max_length];

            boost::system::error_code error;
            size_t length = sock.read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            boost::asio::write(sock, boost::asio::buffer(data, length));

            float temp, hum;
            int send_interval, co2, voc, pm25, pm10, lux, cct;

            data[length] = 0;
            int ret = sscanf(data, "%d|%f,%f,%d,%d,%d,%d,%d,%d,%*d,%*d,%*d", &send_interval, &temp, &hum, &co2, &voc, &pm25, &pm10, &lux, &cct);
            if (ret == 9)
            {
                bool is_open = DB_Open();
                if (is_open)
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
                    DB_ExecuteQuery(query);


                    snprintf(query, sizeof(query), "INSERT INTO data(sensor_id, temp, hum, co2, voc, pm25, pm10, lux, cct, time) VALUES(1, %d, %d, %d, %d, %d, %d, %d, %d, strftime('%%s', 'now'))",
                        (int)roundf(temp * 10.f), (int)roundf(hum * 10.f), co2, voc, pm25, pm10, lux, cct);
                    DB_ExecuteQuery(query);

                    sqlite3_close(db);

                    FILE* f = fopen("graph.html", "w");
                    //fwrite(str_page_1, sizeof(char), sizeof(str_page_1), f);
                    //fwrite(str_page_2, sizeof(char), sizeof(str_page_2), f);
                    fclose(f);
                }

            }
            else
            {
                //Logger::Get()->Log(warning, "Invalid data received from %s", sock.remote_endpoint().address().to_string().c_str());
            }

            //boost::asio::write(sock, boost::asio::buffer(data, length));
        }
    }
    catch (std::exception& e)
    {
        LOGMSG(warning, "Excpetion in thread %s", e.what());
    }
}

void server(boost::asio::io_context& io_context, unsigned short port)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        std::thread(session, a.accept()).detach();
    }
}

void tcp_thread(int port)
{
    boost::asio::io_context io_context;
    server(io_context, port);
}


static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


void Sensors::Init(void)
{
    bool is_open = DB_Open();
    if (is_open)
    {
        char* query = (char*)"SELECT* FROM(SELECT rowid, *FROM data ORDER BY rowid DESC LIMIT 3) ORDER BY rowid ASC";
        DB_ExecuteQuery(query, callback);
    }

    std::thread(tcp_thread, tcp_port).detach();
}

#if 0

// Declare the supported options.
po::options_description desc("Allowed options");
desc.add_options()
("help", "produce help message")
("p", po::value<int>(), "set port")
;

po::variables_map vm;
po::store(po::parse_command_line(argc, argv, desc), vm);
po::notify(vm);

if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
}

int port = vm.count("p") ? vm["p"].as<int>() : 7777;
#endif