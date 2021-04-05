#include "utils/AsyncSerial.h"

#include "CustomMacro.h"
#include "Logger.h"

#include "utils/crc16.h"

using namespace std::chrono_literals;

void CustomMacro::PressKey(char key_to_press)
{
    INPUT ip;  // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    if (isupper(key_to_press))
    {
        INPUT Input = { 0 };  // shift key down
        Input.type = INPUT_KEYBOARD;
        Input.ki.wVk = VK_LSHIFT;
        SendInput(1, &Input, sizeof(INPUT));
    }
    ip.ki.wVk = VkKeyScanExA(key_to_press, GetKeyboardLayout(0));  // Press the "A" key
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));
    if (isupper(key_to_press))  // shift key release
    {
        INPUT Input = { 0 };
        Input.type = INPUT_KEYBOARD;
        Input.ki.dwFlags = KEYEVENTF_KEYUP;
        Input.ki.wVk = VK_LSHIFT;
        SendInput(1, &Input, sizeof(INPUT));
    }
    ip.ki.dwFlags = KEYEVENTF_KEYUP; //  Release the "A" key - KEYEVENTF_KEYUP for key release
    SendInput(1, &ip, sizeof(INPUT));
}

void CustomMacro::UartDataReceived(const char* data, unsigned int len)
{
    //printf("data recv %c", data[0]);
    KeyData_t* k = (KeyData_t*)data;
    uint16_t crc = crc16_calculate((uint8_t*)data, 1);
    if (k->crc == crc)
    {
        const auto it = key_map.find(k->key);
        if (it != key_map.end())
        {
            for(const auto& i : it->second)
            {
                i->DoWrite();
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
                //Logger::Get()->Log(error, "Serial port unexpectedly closed");
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
