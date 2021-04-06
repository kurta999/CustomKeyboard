#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"

#include <inttypes.h>
#include <map>
#include <string>

#include "Logger.h"

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
    void PressReleaseKey(char key, bool press = true)
    {
        INPUT Input = { 0 };
        Input.type = INPUT_KEYBOARD;
        if(!press)  /* relase */
            Input.ki.dwFlags = KEYEVENTF_KEYUP;
        Input.ki.wVk = key;
        SendInput(1, &Input, sizeof(INPUT));
    }
protected:
    std::vector < uint16_t> seq; /* virtual key codes to press and release*/
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
            PressReleaseKey(seq[i]);
            PressReleaseKey(seq[i], false);
        }
    }
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
    std::map<char, std::vector<KeyClass*>> key_vec;
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

    inline uint16_t GetKeyCode(char from_char)
    {
        return VkKeyScanExA(from_char, GetKeyboardLayout(0));
    }
    uint16_t GetSpecialKeyCode(std::string str)
    {
        uint16_t ret = 0xFFFF;
        static const std::map<std::string, int> special_keys =
        {
            {"LCTRL",       VK_LCONTROL},
            {"RCTRL",       VK_RCONTROL},
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
            const char ch = tolower(str.at(0));
            if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
                ret = GetKeyCode(ch);
        }
        else
        {
            auto it = special_keys.find(str);
            if(it != special_keys.end())
                ret = it->second;
        }
        return ret;
    }

    std::vector<MacroContainer*> macros;
    uint16_t com_port = 2005;
    bool use_per_app_macro;
};