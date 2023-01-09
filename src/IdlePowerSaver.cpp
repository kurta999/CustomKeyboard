#include "pch.hpp"

constexpr uint8_t MIN_CPU_PERCENT_POWER_SAVE = 0;
constexpr uint8_t MIN_CPU_PERCENT_PERFORMANCE = 100;
constexpr int64_t CPU_USAGE_SAMPLE_RATE = 250;  /* Unit: milliseconds */

static float GetCPULoad();

#ifdef _WIN32
static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
    static unsigned long long _previousTotalTicks = 0;
    static unsigned long long _previousIdleTicks = 0;

    unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
    unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

    float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

    _previousTotalTicks = totalTicks;
    _previousIdleTicks = idleTicks;
    return ret;
}

static unsigned long long FileTimeToInt64(const FILETIME& ft)
{
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}
#endif

// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
// You'll need to call this at regular intervals, since it measures the load between
// the previous call and the current one.  Returns -1.0 on error.
float GetCPULoad()
{
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
#else
    return 0.0f;
#endif
}

IdlePowerSaver::IdlePowerSaver()
{
    m_powerPercents.reserve(MAX_CPU_POWER_SAVER_QUEUE_SIZE);
}

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
                if(median_load < min_load_threshold)
                {
                    LOG(LogLevel::Normal, "Limit CPU frequency");
                    SetCpuPowerPercent(MIN_CPU_PERCENT_POWER_SAVE, reduced_power_percent);
                    is_power_reduced = true;
                }
                else
                {
                    LOG(LogLevel::Normal, "Median load too big, limit won't be applied: {} (threshold: {})", median_load, min_load_threshold);
                }
            }
        }
        else if(is_power_reduced && idle_time < (timeout - timeout_hystheresis))
        {
            LOG(LogLevel::Normal, "Restore CPU frequency");
            SetCpuPowerPercent(MIN_CPU_PERCENT_PERFORMANCE, 100);
            is_power_reduced = false;
        }

        int64_t dif = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_lastExec).count();
        if(dif > CPU_USAGE_SAMPLE_RATE)
        {
            const float raw_cpu_usage = GetCPULoad();
            uint8_t real_cpu_usage = static_cast<uint8_t>(std::round(raw_cpu_usage * 100.f));
            if(m_powerPercents.size() >= MAX_CPU_POWER_SAVER_QUEUE_SIZE)
            {
                //LOG(LogLevel::Notification, "Pre Vec: {}, {}, {}, {}, {}", m_powerPercents[0], m_powerPercents[1], m_powerPercents[2], m_powerPercents[3], m_powerPercents[4]);
                std::rotate(m_powerPercents.begin(), m_powerPercents.begin() + 1, m_powerPercents.end());
                //LOG(LogLevel::Notification, "Post Vec: {}, {}, {}, {}, {}", m_powerPercents[0], m_powerPercents[1], m_powerPercents[2], m_powerPercents[3], m_powerPercents[4]);
                m_powerPercents.back() = real_cpu_usage;
            }
            else
                m_powerPercents.push_back(real_cpu_usage);
            
            m_lastExec = std::chrono::steady_clock::now();

            if(m_powerPercents.size() & 1)
            {
                assert("m_powerPercents size is odd! That's should never happen!");
            }
            else
            {
                decltype(m_powerPercents) copy_vec;
                std::copy(m_powerPercents.begin(), m_powerPercents.end(), std::back_inserter(copy_vec));
                const auto median_it = copy_vec.begin() + copy_vec.size() / 2;
                std::nth_element(copy_vec.begin(), median_it, copy_vec.end());
                median_load = *median_it;
                //LOG(LogLevel::Notification, "Median CPU: {}, {}", real_cpu_usage, median_load);
            }

            if(is_power_reduced && median_load > max_load_threshold)
            {
                SetCpuPowerPercent(MIN_CPU_PERCENT_PERFORMANCE, 100);
                is_power_reduced = false;
                LOG(LogLevel::Normal, "Restore CPU frequency - too much load: {} (threshold: {})", median_load, max_load_threshold);
            }
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
#ifdef _WIN32
    DWORD percent = std::numeric_limits<uint8_t>::max();
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
    return static_cast<uint8_t>(percent);
#else
    return 0;
#endif
}

