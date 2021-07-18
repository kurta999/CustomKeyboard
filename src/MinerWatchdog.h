#pragma once

#include "utils/CSingleton.h"
#include <string>
#include <Windows.h>
#include <tlhelp32.h>

class MinerWatchdog : public CSingleton < MinerWatchdog >
{
    friend class CSingleton < MinerWatchdog >;
public:
    MinerWatchdog();
    ~MinerWatchdog();

    void CheckProcessRunning();
    void CheckOverclockErrors();
    void DeleteHandles();
    std::string miner_dir;
    std::string miner_params;
private:
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
};

namespace utils
{
    bool IsProcessRunning(const wchar_t* processName);
    void KillProcessByName(const wchar_t* filename);
}