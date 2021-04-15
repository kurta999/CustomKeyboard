#include "utils/AsyncSerial.h"

#include "CustomMacro.h"
#include "Logger.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>

using namespace std::chrono_literals;
using crc16_modbus_t = boost::crc_optimal<16, 0x8005, 0xFFFF, 0, true, true>;

void CustomMacro::UartDataReceived(const char* data, unsigned int len)
{
    KeyData_t* k = (KeyData_t*)data;
    crc16_modbus_t calc_result;
    calc_result.process_byte((uint8_t)data[0]);
    uint16_t crc = calc_result.checksum();
    if(k->crc == crc)
    {
        if(use_per_app_macro)
        {
            HWND foreground = GetForegroundWindow();
            if(foreground)
            {
                char window_title[256];
                GetWindowTextA(foreground, window_title, 256);
                DBG("Focus: %s\n", window_title);
                for(auto& m : macros)
                {
                    if(boost::algorithm::contains(window_title, m->name) && m->name.length() > 2)
                    {
                        const auto it = m->key_vec.find(k->key);
                        if(it != m->key_vec.end())
                        {
                            for(const auto& i : it->second)
                            {
                                i->DoWrite();
                            }
                        }
                        return; /* Exit from loop */
                    }
                }
            }
        }
        else
        {
            const auto it = macros[0]->key_vec.find(k->key);
            if(it != macros[0]->key_vec.end())
            {
                for(const auto& i : it->second)
                {
                    i->DoWrite();
                }
            }

        }
    }
}

void CustomMacro::Init(void)
{
    std::thread(&CustomMacro::WorkerThread, this).detach();
}

void CustomMacro::WorkerThread(void)
{
    try
    {
        CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 921600); /* baud rate has no meaning here */
        auto fp = std::bind(&CustomMacro::UartDataReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial.setCallback(fp);

        for (;;)
        {
            if (serial.errorStatus() || serial.isOpen() == false)
            {
                LOGMSG(error, "Serial port unexpectedly closed");
                break;
            }
            std::this_thread::sleep_for(1ms);
        }
        serial.close();
    }
    catch (std::exception& e)
    {
        LOGMSG(error, "Exception %s", e.what());
    }

}
