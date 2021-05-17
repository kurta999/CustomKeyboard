#pragma once

#include "utils/CSingleton.h"
#include "Logger.h"
#include "Database.h"

#include <deque>

#pragma pack(push, 1)
class Measurement
{
public:
    enum
    {
        TEMPERATURE, HUMIDITY, CO2, VOC, PM25, PM10, LUX, CCT, TIME
    };

    Measurement(float _temp, float _hum, int _co2, int _voc, int _pm25, int _pm10, int _lux, int _cct, std::string&& time_) :
        temp(_temp), hum(_hum), co2(_co2), voc(_voc), pm25(_pm25), pm10(_pm10), lux(_lux), cct(_cct), time(std::move(time_))
    {

    }

    float temp, hum;
    int send_interval = 0, co2, voc, pm25, pm10, lux, cct;
    std::string time;
};
#pragma pack(pop)

constexpr size_t MAX_MEAS_QUEUE = 100;

class Sensors : public CSingleton < Sensors >
{
    friend class CSingleton < Sensors >;
public:
    void Init();
    void ProcessIncommingData(char* recv_data, const char* from_ip);
    void WriteGraphs();

    void AddMeasurement(std::shared_ptr<Measurement>& meas)
    {
        size_t size = last_meas.size();
        if(size >= MAX_MEAS_QUEUE)
            last_meas.pop_front();

        last_meas.push_back(meas);
    }
    inline void AddMeasurementDay(std::shared_ptr<Measurement>& meas)
    {
        last_day.push_back(meas);
    }
    inline void AddMeasurementWeek(std::shared_ptr<Measurement>& meas)
    {
        last_week.push_back(meas);
    }

    const std::deque<std::shared_ptr<Measurement>>& GetMeasurements()
    {
        return last_meas;
    }

private:
    /* TODO: one day merge these functions with more template specialization */
    template<typename T1> void WriteGraphQueue(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1);
    template<typename T1, typename T2> void WriteGraphVector(const char* filename, const char* first_name, const char* second_name, size_t offset_1, size_t offset_2, std::vector<std::shared_ptr<Measurement>>& vec);

    std::deque<std::shared_ptr<Measurement>> last_meas;
    std::vector<std::shared_ptr<Measurement>> last_day;
    std::vector<std::shared_ptr<Measurement>> last_week;
    std::vector<std::shared_ptr<Measurement>> custom;

    std::string template_str;
};
