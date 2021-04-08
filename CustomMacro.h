#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"

#include <inttypes.h>
#include <map>
#include <string>

#include "Logger.h"
#include <thread>

#pragma pack(push, 1)
typedef struct
{
    char key;
    uint16_t crc;
} KeyData_t;
#pragma pack(pop)

class KeyClass
{
public:
    virtual void DoWrite(void) = 0;

public:
    void PressReleaseKey(uint16_t key, bool press = true, bool shift_for_capital_letters = false)
    {
        INPUT Input = { 0 };
        if((key & (1 << 15)) && shift_for_capital_letters)  /* press shift if the key is uppercase */
        {
            Input.type = INPUT_KEYBOARD;
            Input.ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP;
            Input.ki.wVk = VK_LSHIFT;
            SendInput(1, &Input, sizeof(INPUT));
        }
        Input.type = INPUT_KEYBOARD;
        Input.ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP;
        Input.ki.wVk = key & 0x7FFF;
        SendInput(1, &Input, sizeof(INPUT));
    }
protected:
};

class KeyText : public KeyClass
{
public:
    KeyText(std::vector<uint16_t>&& keys)
    {
        seq = std::move(keys);
    }

    void DoWrite()
    {
        for(size_t i = 0; i < seq.size(); i++)
        {
            PressReleaseKey(seq[i], true, true);
            PressReleaseKey(seq[i], false, true);
        }
    }
private:
    std::vector<uint16_t> seq; /* virtual key codes to press and release*/
};

class KeyCombination : public KeyClass
{
public:
    KeyCombination(std::vector<uint16_t>&& keys)
    {
        seq = std::move(keys);
    }
    void DoWrite()
    {
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i]);
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i], false);
    }
private:
    std::vector<uint16_t> seq; /* virtual key codes to press and release*/

};
// teszt szöveg
class KeyDelay : public KeyClass
{
public:
    KeyDelay(uint32_t delay) : delay_ms(delay)
    {
        
    }

    void DoWrite()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
private:
    uint32_t delay_ms;
};

/* each given macro per-app get's a macro container */
class MacroContainer
{
public:
    MacroContainer() = default;
    template <class T>
    MacroContainer(std::string& name, T* p)
    {
        //key_vec.push_back(p);
    }
    std::map<char, std::vector<std::unique_ptr<KeyClass>>> key_vec;
    std::string name;
private:
};

//class Settings;
class CustomMacro : public CSingleton < CustomMacro >
{
    friend class CSingleton < CustomMacro >;

public:
    CustomMacro() = default;
    //~CustomMacro() = delete;
    void Init(void);

private:
    friend class Settings;
    void PressKey(char key_to_press);
    void UartDataReceived(const char* data, unsigned int len);
    void WorkerThread(void);

    uint16_t GetKeyCode(char from_char)
    {
        uint16_t ret = VkKeyScanExA(from_char, GetKeyboardLayout(0));
        if(isupper((unsigned char)from_char))
            ret |= 1 << 15;
        return ret;
    }
    uint16_t GetSpecialKeyCode(std::string str)
    {
        uint16_t ret = 0xFFFF;
        static const std::map<std::string, int> special_keys =
        {
            {"LCTRL",       VK_LCONTROL},
            {"RCTRL",       VK_RCONTROL},
            {"ALT",         VK_MENU},
            {"LSHIFT",      VK_LSHIFT},
            {"RSHIFT",      VK_RSHIFT},
            {"BACKSPACE",   VK_BACK},
            {"TAB",         VK_TAB},
            {"ENTER",       VK_RETURN},
            {"ESC",         VK_ESCAPE},
            {"ESC",         VK_CAPITAL},
            {"SPACE",       VK_SPACE},
            {"PAGEUP",      VK_PRIOR},
            {"PAGEDOWN",    VK_NEXT},
            {"END",         VK_END},
            {"HOME",        VK_HOME},
            {"LEFT",        VK_LEFT},
            {"UP",          VK_UP},
            {"RIGHT",       VK_RIGHT},
            {"DOWN",        VK_DOWN},
            {"PRINT",       VK_SNAPSHOT},
            {"INSERT",      VK_INSERT},
            {"DELETE",      VK_DELETE},
        };
        
        if(str.length() == 1)
        {
            const char ch = str[0];
            if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
                ret = GetKeyCode(tolower(str[0]));
        }
        else
        {
            auto it = special_keys.find(str);
            if(it != special_keys.end())
                ret = it->second;
        }
        return ret;
    }

    std::vector<std::unique_ptr<MacroContainer>> macros;
    uint16_t com_port = 2005;
    bool use_per_app_macro;
};