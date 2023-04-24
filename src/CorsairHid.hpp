#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <thread>
#include <memory>

#ifdef USE_HIDAPI
extern "C"
{
#include "hidapi/hidapi.h"
}
#endif

class CorsairHid : public CSingleton < CorsairHid >
{
    friend class CSingleton < CorsairHid >;

public:
    CorsairHid() = default;
    ~CorsairHid();

    // !\brief Initialize HID polling for Corsair G-Keys
    bool Init();

    // !\brief Set debouncing interval
    // !\param interval Debouncing interval [ms
    void SetDebouncingInterval(const uint16_t interval);

    // !\brief Get debouncing interval [ms]
    // !\return Debouncing interval [ms]
    uint16_t GetDebouncingInterval() const;

private:
    // !\brief Execute init sequence for HID
    // !\details Initialization can't be in main thread because sometime hid_open_path take more than 1 minute to finish
    //            and entrie app will be blocked
    bool ExecuteInitSequence();

    // !\brief Destroys working thread
    void DestroyWorkingThread();

    // !\brief Thread function
    void ThreadFunc(std::stop_token token);

    // !\brief Handle keypress (like bounce checking)
    void HandleKeypress(const std::string& key);

    // !\brief Is HID inited?
    bool hid_inited = false;

#ifdef USE_HIDAPI
    // !\brief HID Handle
    hid_device* hid_handle = nullptr;
#endif

    // !\brief Timepoint when the key was pressed
    std::chrono::steady_clock::time_point last_keypress;

    // !\brief Debouncing interval
    uint16_t m_DebouncingInterval = 350;

    // !\brief Pointer to worker thread
    std::unique_ptr<std::jthread> m_worker = nullptr;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;

    // !\brief Mutex for conditional variable
    std::mutex m_Mutex;

    // !\brief Map with G-Key values and it's name
    const std::map<int, std::string> corsair_GKeys =
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
};