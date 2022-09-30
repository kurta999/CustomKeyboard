#pragma once

#include "utils/CSingleton.hpp"
#include <inttypes.h>

class IdlePowerSaver : public CSingleton < IdlePowerSaver >
{
    friend class CSingleton < IdlePowerSaver >;

public:
    IdlePowerSaver() = default;
    ~IdlePowerSaver();

    // \brief Process for idle power saver
    // \param idle_time [in] Idle time [ms]
    void Process(uint32_t idle_time);

    // \brief Is enabled?
    bool is_enabled = false;

    // \brief Required idle time before activating power saver [s]
    uint32_t timeout = 300;

    // \brief Desirable cpu power percent when computer is idle 
    uint8_t reduced_power_percent = 98;

private:
    // \brief Set CPU min-max power percent
    // \param min_percent [in] Desired min new power percent [0 - 100]
    // \param max_percent [in] Desired max new power percent [0 - 100]
    void SetCpuPowerPercent(uint8_t min_percent, uint8_t max_percent);

    // \brief Get CPU Max power percent
    // \return CPU Power percent [0 - 100], 0xFF is error occurred
    uint8_t GetCpuMaxPowerPercent();

    // \brief Timeout hysteresis [ms]
    const int timeout_hystheresis = 2000;

    // \brief Is CPU frequency reduced?
    bool is_power_reduced = false;
};