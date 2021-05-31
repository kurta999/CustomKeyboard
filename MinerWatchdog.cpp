#include "MinerWatchdog.h"

#include "Logger.h"

#include <thread>
#include <array>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <boost/algorithm/string.hpp>

#include "CustomMacro.h"

MinerWatchdog::MinerWatchdog()
{
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
}

MinerWatchdog::~MinerWatchdog()
{
    DeleteHandles();
}

void MinerWatchdog::DeleteHandles()
{
    if(pi.hProcess != NULL)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
}

void MinerWatchdog::CheckProcessRunning()
{
    BOOL ok = TRUE;
    bool ret = utils::IsProcessRunning(L"ethminer.exe");
    if(!ret)
    {
        DeleteHandles();

        remove(std::string(miner_dir + "log.txt").c_str());
        if(!CreateProcessA(std::string(miner_dir + "ethminer.exe").c_str(), const_cast<char*>(miner_params.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            LOGMSG(error, "CreateProcess failed, error code: {}\n", GetLastError());
            throw 1;
        }

        DBG("ok");

        //CustomMacro::Get()->PressKey(std::string("AFTERBURNER"));
    }
}

void MinerWatchdog::CheckOverclockErrors()
{
    std::stringstream ss;
    std::ifstream f(std::string(miner_dir + "log.txt"), std::ios::in | std::ios::binary);
    if(f.is_open())
    {
        ss << f.rdbuf();
        f.close();

        std::string const& s = ss.str();
        if(s.size() % sizeof(wchar_t) != 0)
        {
            std::cerr << "file not the right size\n";
            return;
        }
        std::wstring ws;
        ws.resize(s.size() / sizeof(wchar_t));
        std::memcpy(&ws[0], s.c_str(), s.size());

        std::vector<std::string> result;
        boost::algorithm::find_all(result, ws, "block");
        if(result.size() > 10)
        {
            utils::KillProcessByName(L"ethimer.exe");
        }
    }
}

namespace utils
{
    bool IsProcessRunning(const wchar_t* processName)
    {
        bool exists = false;
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        if(Process32First(snapshot, &entry))
            while(Process32Next(snapshot, &entry))
                if(!_wcsicmp(entry.szExeFile, processName))
                    exists = true;

        CloseHandle(snapshot);
        return exists;
    }

    void KillProcessByName(const wchar_t* filename)
    {
        HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
        PROCESSENTRY32 pEntry;
        pEntry.dwSize = sizeof(pEntry);
        BOOL hRes = Process32First(hSnapShot, &pEntry);
        while(hRes)
        {
            if(_wcsicmp(pEntry.szExeFile, filename) == 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                    (DWORD)pEntry.th32ProcessID);
                if(hProcess != NULL)
                {
                    TerminateProcess(hProcess, 9);
                    CloseHandle(hProcess);
                }
            }
            hRes = Process32Next(hSnapShot, &pEntry);
        }
        CloseHandle(hSnapShot);
    }
}