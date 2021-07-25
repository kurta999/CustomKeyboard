#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"

#include <inttypes.h>
#include <unordered_map>
#include <map>
#include <string>
#include <variant>
#include <array>

#include "Logger.h"
#include <thread>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#pragma pack(push, 1)
typedef struct
{
    uint8_t state;
    uint8_t lctrl;
    uint8_t lshift;
    uint8_t lalt;
    uint8_t lgui;
    uint8_t rctrl;
    uint8_t rshift;
    uint8_t ralt;
    uint8_t rgui;
    uint8_t keys[6];
    uint16_t crc;
} KeyData_t;
#pragma pack(pop)

class KeyClass
{
public:
    KeyClass() {}
    virtual ~KeyClass() {}
    KeyClass(const KeyClass&) {}
    virtual void DoWrite(void) { };
    virtual KeyClass* Clone() = 0;

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

    void PressReleaseMouse(WORD mouse_button)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = mouse_button;
        SendInput(1, &input, sizeof(input));
        input.mi.dwFlags = mouse_button << (uint16_t)1;
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
    KeyText(const KeyText& from)
    {
        seq = from.seq;
    }
    virtual ~KeyText() { }
    KeyText* Clone() override
    {
        return new KeyText(*this);
    }

    void DoWrite() override
    {
        for(size_t i = 0; i < seq.length(); i++)
        {
            TypeCharacter(seq[i] & 0xFF);
        }
    }

    std::string& GetString()
    {
        return seq;
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
    KeyCombination(const KeyCombination& from)
    {
        seq = from.seq;
    }
    virtual ~KeyCombination() { }
    KeyCombination* Clone() override
    {
        return new KeyCombination(*this);
    }

    void DoWrite() override
    {
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i]);
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i], false);
    }

    std::vector<uint16_t>& GetVec()
    {
        return seq;
    }
private:
    std::vector<uint16_t> seq; /* scan codes to press and release*/
};

class KeyDelay final : public KeyClass
{
public:
    KeyDelay(uint32_t delay_) : delay(delay_)
    {
        
    }    
    KeyDelay(uint32_t start_, uint32_t end_) : delay(std::array<uint32_t, 2>{start_, end_})
    {
        //delay = ;
    }
    KeyDelay(const KeyDelay& from)
    {
        delay = from.delay;
    }
    virtual ~KeyDelay() { }
    KeyDelay* Clone() override
    {
        return new KeyDelay(*this);
    }

    void DoWrite() override
    {
        if(std::holds_alternative<uint32_t>(delay))
            std::this_thread::sleep_for(std::chrono::milliseconds(std::get<uint32_t>(delay)));
        else
        {
            std::array<uint32_t, 2> d = std::get<std::array<uint32_t, 2>>(delay);
            boost::uniform_int<> dist(d[0], d[1]);
            boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(gen, dist);

            int ret = die();
            std::this_thread::sleep_for(std::chrono::milliseconds(ret));
        }
    }

    std::variant<uint32_t, std::array<uint32_t, 2>>& GetDelay()
    {
        return delay;
    }
private:
    std::variant<uint32_t, std::array<uint32_t, 2>> delay;
    boost::mt19937 gen;
};

class MouseMovement final : public KeyClass
{
public:
    MouseMovement(LPPOINT* pos_)
    {
        memcpy(&pos, pos_, sizeof(pos));
    }
    MouseMovement(const MouseMovement& from)
    {
        memcpy(&pos, &from.pos, sizeof(pos));
    }
    virtual ~MouseMovement() { }
    MouseMovement* Clone() override
    {
        return new MouseMovement(*this);
    }

    void DoWrite() override
    {
        POINT to_screen;
        HWND hwnd = GetForegroundWindow();
        memcpy(&to_screen, &pos, sizeof(to_screen));
        ClientToScreen(hwnd, &to_screen);
        ShowCursor(FALSE);
        SetCursorPos(to_screen.x, to_screen.y);
        ShowCursor(TRUE);
    }

    POINT& GetPos()
    {
        return pos;
    }

private:
    POINT pos;
};

class MouseClick final : public KeyClass
{
public:
    MouseClick(uint16_t key_) : key(key_)
    {}
    MouseClick(const MouseClick& from)
    {
        key = from.key;
    }
    virtual ~MouseClick() { }
    MouseClick* Clone() override
    {
        return new MouseClick(*this);
    }

    void DoWrite() override
    {
        PressReleaseMouse(key);
    }

    uint16_t GetKey()
    {
        return key;
    }
private:
    uint16_t key;
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
    std::map<std::string, std::vector<std::unique_ptr<KeyClass>>> key_vec;  /* std::string -> it could be better, like storing hash, but I did it for myself - it's OK */
    std::map<std::string, std::string> bind_name;  /* [Key code] = bind name text */
    std::string name;
private:
};

class CustomMacro : public CSingleton < CustomMacro >
{
    friend class CSingleton < CustomMacro >;

public:
    CustomMacro() = default;
    ~CustomMacro()
    {
        if(t)
            TerminateThread(t->native_handle(), 0);
    }
    void Init(void);
    std::vector<std::unique_ptr<MacroContainer>>& GetMacros()
    {
        return macros;
    }

    uint16_t GetKeyScanCode(const std::string& str)
    {
        uint16_t ret = 0xFFFF;
        auto it = scan_codes.find(str);
        if(it != scan_codes.end())
            ret = it->second;
        return ret;
    }

    std::string GetKeyStringFromScanCode(int scancode)
    {
        std::string ret = "INVALID";
        for(auto& i : scan_codes)
        {
            if(i.second == scancode)
            {
                ret = i.first;
                break;
            }
        }
        return ret;
    }

private:
    friend class Settings;
    friend class MinerWatchdog;

    void PressKey(std::string key);
    void UartDataReceived(const char* data, unsigned int len);
    void UartReceiveThread(void);

    std::vector<std::unique_ptr<MacroContainer>> macros;
    uint8_t is_enabled;
    uint16_t com_port = 2005;
    bool use_per_app_macro;
    bool advanced_key_binding;
    std::string pressed_keys;
    std::thread* t = nullptr;
    static const std::unordered_map<std::string, int> scan_codes;
    static const std::unordered_map<int, std::string> hid_scan_codes;
};