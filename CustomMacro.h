#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"

#include <inttypes.h>
#include <unordered_map>
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
    void PressReleaseKey(uint16_t scancode, bool press = true)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = scancode;
        input.ki.dwFlags = (press ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_SCANCODE;
        if((scancode & 0xFF00) == 0xE000)
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        SendInput(1, &input, sizeof(input));
    }
  
    void TypeCharacter(WORD character)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = character;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        if((character & 0xFF00) == 0xE000)
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        SendInput(1, &input, sizeof(input));
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(input));
    }
protected:
};

class KeyText final : public KeyClass
{
public:
    KeyText(std::string&& keys)
    {
        seq = std::move(keys);
    }

    void DoWrite()
    {
        for(size_t i = 0; i < seq.length(); i++)
        {
            TypeCharacter(seq[i] & 0xFF);
        }
    }
private:
    std::string seq; /* virtual key codes to press and release*/
};

class KeyCombination final : public KeyClass
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
    std::vector<uint16_t> seq; /* scan codes to press and release*/

};

class KeyDelay final : public KeyClass
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
    void UartDataReceived(const char* data, unsigned int len);
    void WorkerThread(void);

    uint16_t GetKeyScanCode(std::string str)
    {
        uint16_t ret = 0xFFFF;
        static const std::unordered_map<std::string, int> scan_codes =
        {
            {"LCTRL",       0x1D},
            {"RCTRL",       0xE01D},
            {"LALT",        0x38},
            {"RALT",        0xE038},
            {"LSHIFT",      0x2A},
            {"RSHIFT",      0x36},
            {"BACKSPACE",   0xE0},
            {"TAB",         0x0F},
            {"ENTER",       0x1C},
            {"ESC",         0x01},
            {"SPACE",       0x39},
            {"PAGEUP",      0xE049},
            {"PAGEDOWN",    0xE051},
            {"END",         0xE04F},
            {"HOME",        0xE047},
            {"PRINT",       0x0}, // TODO
            {"INSERT",      0xE052},
            {"DELETE",      0xE053},
            {"NUM_1",       0x4F},
            {"NUM_2",       0x50},
            {"NUM_3",       0x51},
            {"NUM_4",       0x4B},
            {"NUM_5",       0x4C},
            {"NUM_6",       0x4D},
            {"NUM_7",       0x47},
            {"NUM_8",       0x48},
            {"NUM_9",       0x49},
            {"NUM_0",       0x52},
            {"NUM_MUL",     0x37},
            {"NUM_DOT",     0x53},
            {"NUM_PLUS",    0x4E},
            {"NUM_MINUS",   0x4A},
            {"NUM_DIV",     0xE035},
            {"A",           0x1E},
            {"B",           0x30},
            {"C",           0x2E},
            {"D",           0x20},
            {"E",           0x12},
            {"F",           0x21},
            {"G",           0x22},
            {"H",           0x23},
            {"I",           0x17},
            {"J",           0x24},
            {"K",           0x25},
            {"L",           0x26},
            {"M",           0x32},
            {"N",           0x31},
            {"O",           0x18},
            {"P",           0x19},
            {"Q",           0x10},
            {"R",           0x13},
            {"S",           0x1F},
            {"T",           0x14},
            {"U",           0x16},
            {"V",           0x2F},
            {"W",           0x11},
            {"X",           0x2D},
            {"Y",           0x15},
            {"Z",           0x2C},
            {"1",           0x02},
            {"2",           0x03},
            {"F1",          0x3B},
            {"F2",          0x3C},
            {"F3",          0x3D},
            {"F4",          0x3E},
            {"F5",          0x3F},
            {"F6",          0x40},
            {"F7",          0x41},
            {"F8",          0x42},
            {"F9",          0x43},
            {"F10",         0x44},
            {"F11",         0x59},
            {"F12",         0x58},
            {"LEFT",        0xE04B},
            {"RIGHT",       0xE04D},
            {"UP",          0xE048},
            {"DOWN",        0xE050},
        };

        auto it = scan_codes.find(str);
        if(it != scan_codes.end())
            ret = it->second;
        return ret;
    }

    std::vector<std::unique_ptr<MacroContainer>> macros;
    uint16_t com_port = 2005;
    bool use_per_app_macro;
};