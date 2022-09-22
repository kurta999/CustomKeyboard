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
    uint32_t timeout = 3;

    // \brief Desirable cpu power percent when computer is idle 
    uint8_t reduced_power_percent = 98;

private:
    // \brief Timeout hysteresis [ms]
    const int timeout_hystheresis = 2000;

    // \brief Is CPU frequency reduced?
    bool is_power_reduced = false;

    // \brief Set CPU power percent
    // \param percent [in] Desired new power percent [0 - 100]
    void SetCpuPowerPercent(uint8_t percent);

    // \brief Get CPU power percent
    // \return CPU Power percent [0 - 100], 0xFF is error occurred
    uint8_t GetCpuPowerPercent();
};