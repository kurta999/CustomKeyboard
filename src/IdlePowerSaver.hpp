#pragma once

#include "utils/CSingleton.hpp"
#include <inttypes.h>

constexpr int64_t MAX_CPU_POWER_SAVER_QUEUE_SIZE = 50;

static_assert((MAX_CPU_POWER_SAVER_QUEUE_SIZE & 1) == 0, "MAX_QUEUE_SIZE has to be even");

class IdlePowerSaver : public CSingleton < IdlePowerSaver >
{
    friend class CSingleton < IdlePowerSaver >;

public:
    IdlePowerSaver();
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

    // \brief Minimum load threshold. Median load must be lower than this in order to activate cpu power saver
    uint8_t min_load_threshold = 50;
    
    // \brief Maximum load threshold. If this value is reached while the cpu power saver is active, it will be disabled and only enables again
    //        when median CPU load is lower than min_load_threshold
    uint8_t max_load_threshold = 80;

    // \brief Set CPU min-max power percent
    // \param min_percent [in] Desired min new power percent [0 - 100]
    // \param max_percent [in] Desired max new power percent [0 - 100]
    void SetCpuPowerPercent(uint8_t min_percent, uint8_t max_percent);

    // \brief Get CPU Min power percent
    // \return CPU Min Power percent [0 - 100], 0xFF is error occurred
    uint8_t GetCpuMinPowerPercent();
    
    // \brief Get CPU Max power percent
    // \return CPU Max Power percent [0 - 100], 0xFF is error occurred
    uint8_t GetCpuMaxPowerPercent();

private:
    // \brief Timeout hysteresis [ms]
    const int timeout_hystheresis = 2000;

    // \brief Is CPU frequency reduced?
    bool is_power_reduced = false;

    // \brief Last CPU power saver process execution
    std::chrono::steady_clock::time_point m_lastExec{};

    // \brief Vector of captured CPU usage from the past
    std::vector<uint8_t> m_powerPercents;

    // \brief Median CPU load [%]
    uint8_t median_load = 0;
};