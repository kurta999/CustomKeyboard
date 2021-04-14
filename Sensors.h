#pragma once

#include "utils/CSingleton.h"
#include "Logger.h"
#include <deque>

class Measurement
{
public:
    Measurement(float _temp, float _hum, int _co2, int _voc, int _pm25, int _pm10, int _lux, int _cct) :
        temp(_temp), hum(_hum), co2(_co2), voc(_voc), pm25(_pm25), pm10(_pm10), lux(_lux), cct(_cct)
    {

    }

    float temp, hum;
    int send_interval = 0, co2, voc, pm25, pm10, lux, cct;
};


class Sensors : public CSingleton < Sensors >
{
    friend class CSingleton < Sensors >;
public:
    void ProcessIncommingData(char* recv_data, const char* from_ip);


private:
    std::deque<std::shared_ptr<Measurement>> last_meas;
};
