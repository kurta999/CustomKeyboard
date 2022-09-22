#include "pch.hpp"

IdlePowerSaver::~IdlePowerSaver()
{
    SetCpuPowerPercent(100);
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
                SetCpuPowerPercent(reduced_power_percent);
                is_power_reduced = true;
            }
        }
        else if(is_power_reduced && idle_time < (timeout - timeout_hystheresis))
        {
            LOG(LogLevel::Normal, "Restore CPU frequency");
            SetCpuPowerPercent(100);
            is_power_reduced = false;
        }
    }
}

void IdlePowerSaver::SetCpuPowerPercent(uint8_t percent)
{
#ifdef _WIN32
    GUID* scheme;
    int error = PowerGetActiveScheme(NULL, &scheme);
    if(error)
    {
        LOG(LogLevel::Error, "PowerGetActiveScheme returned error code: {}", error);
        return;
    }

    if(percent > 100)
        percent = 100;
    DWORD cpu_max_percent = static_cast<DWORD>(percent);
    error = PowerWriteACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, cpu_max_percent);
    if(error)
    {
        LOG(LogLevel::Error, "PowerWriteACValueIndex returned error code: {}", error);
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

uint8_t IdlePowerSaver::GetCpuPowerPercent()
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

