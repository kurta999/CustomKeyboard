#include "pch.hpp"

constexpr int HID_READ_TIMEOUT = 100;
constexpr int READ_DATA_BUFFER_SIZE = 64;
constexpr int MIN_READ_DATA_SIZE = 20;

constexpr int DEBOUNCING_INTERVAL = 250;

CorsairHid::~CorsairHid()
{
    DestroyWorkingThread();
}

bool CorsairHid::Init()
{
    LOG(LogLevel::Notification, "CorsairHid::Init");
    if(hid_inited)
        DestroyWorkingThread();

    if(m_hid_init_future.valid())
        if(m_hid_init_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            return false;
    m_hid_init_future = std::async(&CorsairHid::ExecuteInitSequence, this);
    return true;
}

bool CorsairHid::ExecuteInitSequence()
{
#ifdef USE_HIDAPI
    LOG(LogLevel::Notification, "CorsairHid::ExecuteInitSequence");
    int ret = hid_init();  /* Initialize the hidapi library */
    if(ret)
    {
        LOG(LogLevel::Critical, "hid_init failed");
        return false;
    }

    hid_inited = true;
    const char* hid_path = nullptr;

    hid_device_info* device_info = hid_enumerate(0, 0);  /* Enumerate over all HID devices */
    while(device_info != NULL)
    {
        LOGW(LogLevel::Normal, L"HID Device: \"{}\", VID: 0x{:X}, PID: 0x{:X}, UsagePage: 0x{:X}, Usage: 0x{:X}",
            device_info->product_string, device_info->vendor_id, device_info->product_id, device_info->usage_page, device_info->usage);

        if(device_info->vendor_id == 0x1B1C && device_info->product_id == 0x1B11 && device_info->usage_page == 0xFFC0 && device_info->usage == 2)  /* K95 RGB (older) with 18 macro keys */
        {
            hid_path = device_info->path;
            LOG(LogLevel::Normal, "Corsair K95 18 macro key found");
            break;
        }

        if(std::wstring(device_info->product_string).find(L"Corsair Gaming K95") != std::wstring::npos)  /* Corsair Gaming K95 RGB PLATINUM Keyboard */
        {
            hid_path = device_info->path;
            LOG(LogLevel::Normal, "Corsair K95 Platinum found");
            break;
        }
        device_info = device_info->next;
    }

    if(hid_path != nullptr)
    {
        hid_handle = hid_open_path(hid_path);  /* Never call this function from main thread, it can block occasionally! */
        if(!hid_handle)
        {
            LOG(LogLevel::Critical, "hid_open failed");
            return false;
        }

        m_exit = false;
        m_worker = std::make_unique<std::thread>(&CorsairHid::ThreadFunc, this);
    }
    else
    {
        LOG(LogLevel::Error, "Unable to find Corsair K95");
        return false;
    }
#endif
    return true;
}

void CorsairHid::DestroyWorkingThread()
{
#ifdef USE_HIDAPI
    if(hid_handle)
        hid_close(hid_handle);
    hid_handle = nullptr;
    hid_inited = false;

    bool ret = hid_exit();
#endif
    m_exit = true;
    if(m_worker && m_worker->joinable())
        m_worker->join();
}

void CorsairHid::ThreadFunc()
{
#ifdef USE_HIDAPI
    LOG(LogLevel::Notification, "ThreadFunc");
    while(!m_exit)
    {
        uint8_t recv_data[READ_DATA_BUFFER_SIZE];
        int read_bytes = hid_read_timeout(hid_handle, recv_data, sizeof(recv_data), HID_READ_TIMEOUT);
        if(read_bytes == 0xFFFFFFFF)
        {
            LOGW(LogLevel::Error, L"HID read error: {}", hid_error(hid_handle));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); /* Sleep for one second after error happend */
        }
        else if(read_bytes > MIN_READ_DATA_SIZE)
        {
            uint32_t gkey_code = *reinterpret_cast<uint32_t*>(recv_data + 16);
            auto it = corsair_GKeys.find(gkey_code);
            if(it != corsair_GKeys.end())
            {
                HandleKeypress(it->second);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#endif
}

void CorsairHid::HandleKeypress(const std::string& key)
{
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - last_keypress).count();
    if(elapsed > DEBOUNCING_INTERVAL)
    {
        last_keypress = std::chrono::steady_clock::now();
        CustomMacro::Get()->SimulateKeypress(key);
    }
    else
    {
        LOG(LogLevel::Normal, "Bouncing detected, keypress has been skippped. Elapsed time (ms): {}", elapsed);
    }
}