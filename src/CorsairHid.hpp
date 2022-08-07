#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <thread>
#include <memory>

extern "C"
{
#include "hidapi/hidapi.h"
}

class CorsairHid : public CSingleton < CorsairHid >
{
    friend class CSingleton < CorsairHid >;

public:
    CorsairHid() = default;
    ~CorsairHid();

    bool Init();

private:
    std::map<int, std::string> corsair_GKeys =
    {
        { 1 << 0, "G1" },
        { 1 << 1, "G2" },
        { 1 << 2, "G3" },
        { 1 << 3, "G4" },
        { 1 << 4, "G5" },
        { 1 << 5, "G6" },
        { 1 << 6, "G7" },
        { 1 << 7, "G8" },
        { 1 << 8, "G9" },
        { 1 << 9, "G10" },
        { 1 << 16, "G11" },
        { 1 << 17, "G12" },
        { 1 << 18, "G13" },
        { 1 << 19, "G14" },
        { 1 << 20, "G15" },
        { 1 << 21, "G16" },
        { 1 << 22, "G17" },
        { 1 << 23, "G18" },
    };

    void DestroyWorkingThread();

    void ThreadFunc();

    bool hid_inited = false;
    hid_device* hid_handle = nullptr;
    std::atomic<bool> m_exit = false;
    std::unique_ptr<std::thread> m_worker = nullptr;
};