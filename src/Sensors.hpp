#pragma once

#include "utils/CSingleton.hpp"
#include "Logger.hpp"
#include "DatabaseLogic.hpp"

#include <deque>
#include <memory>
#include <mutex>

#pragma pack(push, 1)
class Measurement
{
public:
    enum
    {
        TEMPERATURE, HUMIDITY, CO2, VOC, PM25, PM10, LUX, CCT, TIME
    };

    Measurement() = default;
    Measurement(float _temp, float _hum, int _co2, int _voc, int _pm25, int _pm10, int _lux, int _cct, std::string&& time_) :
        temp(_temp), hum(_hum), co2(_co2), voc(_voc), pm25(_pm25), pm10(_pm10), lux(_lux), cct(_cct), time(std::move(time_))
    {

    }
    ~Measurement() = default;
    Measurement(const Measurement&) = default;

    float temp, hum;
    int co2, voc, pm25, pm10, lux, cct;
    std::string time;
};
#pragma pack(pop)

constexpr size_t MAX_MEAS_QUEUE = 150;

class Sensors : public CSingleton < Sensors >
{
    friend class CSingleton < Sensors >;
public:
    // \brief Initialize sensors
    void Init();

    // \brief Process incomming data
    // \param recv_data [in] - received data C string
    // \param from_ip [in] - sender ip C string
    bool ProcessIncommingData(const char* recv_data, const char* from_ip);

    // \brief Write graphs from memory to .html files
    void WriteGraphs();

    // \brief Add measurement to last_meas queue
    // \param meas [in] - pointer to measure to add
    void AddMeasurement(std::unique_ptr<Measurement>&& meas);

    // \brief Returns last X measurements
    const std::deque<std::unique_ptr<Measurement>>& GetMeasurements() { return last_meas; }

    // \brief Vector of last day's measurement, array order is: avg, max, min
    std::vector<std::unique_ptr<Measurement>> last_day[3];

    // \brief Vector of last week's measurement, array order is: avg, max, min
    std::vector<std::unique_ptr<Measurement>> last_week[3];

private:
    template<typename T> T GetValueFromDequeue(const std::unique_ptr<Measurement>& meas, int offset);
    template<int i, typename T1, typename T2> int CalculateMinMaxAvg_Final(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<int i, typename T1, typename T2> int CalculateMinMaxAvg(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<typename T1, typename T2> void WriteDataToHtmlFromContainer(std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<typename T1> void WriteGraph(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1);

    // \brief Dequeue for last X measurement
    std::deque<std::unique_ptr<Measurement>> last_meas;

    // \brief Basic HTML template file for measurement graphs
    std::string template_str;

    // \brief Mutex for generating measurements
    std::mutex mtx;

    // \brief Number of received measurements
    size_t num_recv_meas = 0;
};
