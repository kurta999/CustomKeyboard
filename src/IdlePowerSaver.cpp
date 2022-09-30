#include "pch.hpp"

constexpr uint8_t MIN_CPU_PERCENT_POWER_SAVE = 0;
constexpr uint8_t MIN_CPU_PERCENT_PERFORMANCE = 100;

IdlePowerSaver::~IdlePowerSaver()
{
    SetCpuPowerPercent(MIN_CPU_PERCENT_PERFORMANCE, 100);
}

void IdlePowerSaver::Process(uint32_t idle_time)
{
    if(is_enabled)
    {
        idle_time /= 1000;
        if(idle_time > timeout)
        {
            if(!is_power_reduced)
            {
                LOG(LogLevel::Normal, "Limit CPU frequency");
                SetCpuPowerPercent(MIN_CPU_PERCENT_POWER_SAVE, reduced_power_percent);
                is_power_reduced = true;
            }
        }
        else if(is_power_reduced && idle_time < (timeout - timeout_hystheresis))
        {
            LOG(LogLevel::Normal, "Restore CPU frequency");
            SetCpuPowerPercent(MIN_CPU_PERCENT_PERFORMANCE, 100);
            is_power_reduced = false;
        }
    }
}

void IdlePowerSaver::SetCpuPowerPercent(uint8_t min_percent, uint8_t max_percent)
{
#ifdef _WIN32
    GUID* scheme;
    int error = PowerGetActiveScheme(NULL, &scheme);
    if(error)
    {
        LOG(LogLevel::Error, "PowerGetActiveScheme returned error code: {}", error);
        return;
    }

    if(min_percent > 100)
        min_percent = 100;
    DWORD cpu_min_percent = static_cast<DWORD>(min_percent);
    error = PowerWriteACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MINIMUM, cpu_min_percent);
    if(error)
    {
        LOG(LogLevel::Error, "PowerWriteACValueIndex returned error code for GUID_PROCESSOR_THROTTLE_MINIMUM: {}", error);
        return;
    }

    if(max_percent > 100)
        max_percent = 100;
    DWORD cpu_max_percent = static_cast<DWORD>(max_percent);
    error = PowerWriteACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, cpu_max_percent);
    if(error)
    {
        LOG(LogLevel::Error, "PowerWriteACValueIndex returned error code for GUID_PROCESSOR_THROTTLE_MAXIMUM: {}", error);
        return;
    }

    error = PowerSetActiveScheme(NULL, scheme);
    if(error)
    {
        LOG(LogLevel::Error, "PowerSetActiveScheme returned error code: {}", error);
        return;
    }
#endif
}

uint8_t IdlePowerSaver::GetCpuMaxPowerPercent()
{
    DWORD percent = std::numeric_limits<uint8_t>::max();
#ifdef _WIN32
    GUID* scheme;
    int error = PowerGetActiveScheme(NULL, &scheme);
    if(error)
    {
        LOG(LogLevel::Error, "PowerGetActiveScheme returned error code: {}", error);
        return std::numeric_limits<uint8_t>::max();
    }

    error = PowerReadACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, &percent);
    if(error)
    {
        LOG(LogLevel::Error, "PowerReadACValueIndex returned error code: {}", error);
        return std::numeric_limits<uint8_t>::max();
    }
#endif
    return static_cast<uint8_t>(percent);
}

