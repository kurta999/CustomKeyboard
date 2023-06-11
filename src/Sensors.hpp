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

    Measurement(float _temp, float _hum, int _co2, int _voc, int _co, int _pm25, int _pm10, float _pressure, float _r, float _g, float _b, int _lux, int _cct, int _uv, std::string&& time_) :
        temp(_temp), hum(_hum), co2(_co2), voc(_voc), co(_co), pm25(_pm25), pm10(_pm10), pressure(_pressure), r(_r), g(_g), b(_b), lux(_lux), cct(_cct), uv(_uv), time(std::move(time_))
    {

    }
    ~Measurement() = default;
    Measurement(const Measurement&rhs)
    {
        this->operator+=(rhs);
    }

    Measurement& operator =(const Measurement& rhs)
    {
        temp = rhs.temp;
        hum = rhs.hum;
        pressure = rhs.pressure;
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        co2 = rhs.co2;
        voc = rhs.voc;
        co = rhs.co;
        pm25 = rhs.pm25;
        pm10 = rhs.pm10;
        lux = rhs.lux;
        cct = rhs.cct;
        uv = rhs.uv;
        time = rhs.time;
        return *this;
    }

    Measurement& operator +=(const Measurement& rhs)
    {
        temp += rhs.temp;
        hum += rhs.hum;
        pressure += rhs.pressure;
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        co2 += rhs.co2;
        voc += rhs.voc;
        co += rhs.co;
        pm25 += rhs.pm25;
        pm10 += rhs.pm10;
        lux += rhs.lux;
        cct += rhs.cct;
        uv += rhs.uv;
        time += rhs.time;
        cnt++;
        return *this;
    }

    void Finalize()
    {
        temp /= cnt;
        hum /= cnt;
        pressure /= cnt;
        r /= cnt;
        g /= cnt;
        b /= cnt;
        co2 /= cnt;
        voc /= cnt;
        co /= cnt;
        pm25 /= cnt;
        pm10 /= cnt;
        lux /= cnt;
        cct /= cnt;
        uv /= cnt;
        cnt = 1;
    }

    float temp = 0.0f, hum = 0.0f, pressure = 0.0f, r = 0.0f, g = 0.0f, b = 0.0f;
    int co2 = 0, voc = 0, co = 0, pm25 = 0, pm10 = 0, lux = 0, cct = 0, uv = 0;
    std::string time;

    uint8_t cnt = 0;
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
    // \param data_len [in] - received data len
    // \param from_ip [in] - sender ip C string
    bool ProcessIncommingData(const char* recv_data, size_t data_len, const char* from_ip);

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
    enum
    {
        MEAS_INDEX_SCD_TEMP,
        MEAS_INDEX_SCD_HUM,
        MEAS_INDEX_SCD_CO2,
        MEAS_INDEX_CO,
        MEAS_INDEX_BME680_TEMP,
        MEAS_INDEX_BME680_HUM,
        MEAS_INDEX_BME680_PRESSURE,
        MEAS_INDEX_BME680_GAS_RESISTANCE,
        MEAS_INDEX_BME680_TIMESTAMP,
        MEAS_INDEX_PM25,
        MEAS_INDEX_PM10,
        MEAS_INDEX_UV,
        MEAS_INDEX_R,
        MEAS_INDEX_G,
        MEAS_INDEX_B,
        MEAS_INDEX_CCT,
        MEAS_INDEX_Lux,
        MEAS_INDEX_Max
    };

    // \brief Main function for measurement handling
    // \param meas_vec [in] - vector of string containing measurements
    void HandleMeasurements(std::vector<std::string>& meas_vec);

    // \brief Update GUI with given measurements
    // \param m [in] - Measurement object whose params will be printed to the GUI
    void UpdateGui(Measurement& m);

    // \param Update measurement database if specified period elapsed
    void UpdateDatabaseIfNeeded();

    template<typename T> T GetValueFromDequeue(const std::unique_ptr<Measurement>& meas, int offset);
    template<int i, typename T1, typename T2> int CalculateMinMaxAvg_Final(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<int i, typename T1, typename T2> int CalculateMinMaxAvg(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<typename T1, typename T2> void WriteDataToHtmlFromContainer(std::string* labels, std::string* data_values, T2* container, size_t offset);
    template<typename T1> void WriteGraph(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1);

    // \brief Start of current integration period timestamp
    std::chrono::steady_clock::time_point m_IntegrationStartTimestamp;

    // \brief Pointer to current measurement
    std::unique_ptr<Measurement> m_CurrMeas = nullptr;

    // \brief Dequeue for last X measurement
    std::deque<std::unique_ptr<Measurement>> last_meas;

    // \brief Basic HTML template file for measurement graphs
    std::string template_str;

    // \brief Mutex for generating measurements
    std::mutex mtx;

    // \brief Number of received measurements
    size_t num_recv_meas = 0;
};
