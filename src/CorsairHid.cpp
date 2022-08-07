#include "pch.hpp"

constexpr int HID_READ_TIMEOUT = 100;
constexpr int READ_DATA_BUFFER_SIZE = 64;

CorsairHid::~CorsairHid()
{
    DestroyWorkingThread();
}

bool CorsairHid::Init()
{
    if(hid_inited)
        DestroyWorkingThread();

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
        std::string str_log;
        utils::WStringToMBString(device_info->product_string, str_log);
        LOG(LogLevel::Normal, "HID Device: \"{}\", VID: 0x{:X}, PID: 0x{:X}, UsagePage: {:X}, Usage: {:X}\n", 
            str_log.c_str(), device_info->vendor_id, device_info->product_id, device_info->usage_page, device_info->usage);

        if(device_info->vendor_id == 0x1B1C && device_info->product_id == 0x1B11 && device_info->usage_page == 0xFFC0 && device_info->usage == 2)  /* K95 RGB (older) with 18 macro keys */
        {
            hid_path = device_info->path;
            LOG(LogLevel::Normal, "Corsair K95 18 macro key found");
            break;
        }
#if 0
        if(std::wstring(device_info->product_string).find(L"Corsair Gaming K95") != std::string::npos)  /* Corsair Gaming K95 RGB PLATINUM Keyboard */
        {
            hid_path = device_info->path;
            LOG(LogLevel::Normal, "Corsair K95 Platinum found");
            break;
        }
#endif
        device_info = device_info->next;
    }

    if(hid_path != nullptr)
    {
        hid_handle = hid_open_path(hid_path);
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
    return true;
}

void CorsairHid::DestroyWorkingThread()
{
    if(hid_handle)
        hid_close(hid_handle);
    hid_handle = nullptr;
    hid_inited = false;

    bool ret = hid_exit();

    m_exit = true;
    if(m_worker && m_worker->joinable())
        m_worker->join();
}

void CorsairHid::ThreadFunc()
{
    while(!m_exit)
    {
        uint8_t recv_data[READ_DATA_BUFFER_SIZE];
        int read_bytes = hid_read_timeout(hid_handle, recv_data, sizeof(recv_data), HID_READ_TIMEOUT);
        if(read_bytes == 0xFFFFFFFF)
        {
            std::string error_str;
            utils::WStringToMBString(hid_error(hid_handle), error_str);
            LOG(LogLevel::Error, "HID read error: {}", error_str);
        }
        else if(read_bytes > 0)
        {
            uint32_t gkey_code = *reinterpret_cast<uint32_t*>(recv_data + 16); //*(uint32_t*)&recv_data[16];//recv_data[16] | recv_data[17] << 8 | recv_data[18] << 16 | recv_data[19] << 24;
            auto it = corsair_GKeys.find(gkey_code);
            if(it != corsair_GKeys.end())
            {
                CustomMacro::Get()->pressed_keys = it->second;
                CustomMacro::Get()->PressKey(it->second);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}