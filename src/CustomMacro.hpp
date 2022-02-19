#pragma once

#include "utils/CSingleton.hpp"

#include "Settings.hpp"

#include <inttypes.h>
#include <unordered_map>
#include <map>
#include <string>
#include <variant>
#include <array>

#include "Logger.hpp"
#include <thread>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/tokenizer.hpp>

#ifndef _WIN32
typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT, * PPOINT, *NPPOINT, * LPPOINT;
#endif

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

enum MacroTypes : uint8_t
{
    BIND_NAME, KEY_SEQ, KEY_TYPE, DELAY, MOUSE_MOVE, MOUSE_INTERPOLATE, MOUSE_PRESS, MOUSE_RELEASE, MOUSE_CLICK, MAX
};

class IKey
{
public:
    IKey() {}
    virtual ~IKey() {}
    IKey(const IKey&) {}
    virtual void Execute() = 0;
    virtual std::string GenerateText(bool is_ini_format) = 0;
    virtual const char* GetName() = 0;
    virtual IKey* Clone() = 0;
};

class KeyText final : public IKey
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

    void Execute() override
    {
#ifdef _WIN32
        for(size_t i = 0; i < seq.length(); i++)
        {
            TypeCharacter(seq[i] & 0xFF);
        }
#else
        system(fmt::format("xte 'str {}'", seq).c_str());
#endif
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string &&ret = is_ini_format ? fmt::format(" KEY_TYPE[{}]", seq) : seq;
        return ret;
    }
    const char* GetName() override { return name; }

    std::string& GetString()
    {
        return seq;
    }
private:

#ifdef _WIN32
    void TypeCharacter(uint16_t character)
    {
        int count = MultiByteToWideChar(CP_ACP, 0, (char*)&character, 1, NULL, 0);
        wchar_t wide_char;
        MultiByteToWideChar(CP_ACP, 0, (char*)&character, 1, &wide_char, count);
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = wide_char;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        if((wide_char & 0xFF00) == 0xE000)
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        SendInput(1, &input, sizeof(input));
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(input));
    }
#endif
    std::string seq; /* characters to press and release*/
    static inline const char* name = "TEXT";
};

class KeyCombination final : public IKey
{
public:
    KeyCombination(std::vector<uint16_t>&& keys)
    {
        seq = std::move(keys);
    }    
    KeyCombination(std::string&& str);
    KeyCombination(const KeyCombination& from)
    {
        seq = from.seq;
    }
    virtual ~KeyCombination() { }
    KeyCombination* Clone() override
    {
        return new KeyCombination(*this);
    }

    void Execute() override
    {
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i]);
        for(size_t i = 0; i < seq.size(); i++)
            PressReleaseKey(seq[i], false);
    }

    std::string GenerateText(bool is_ini_format) override;
    const char* GetName() override { return name; }

    std::vector<uint16_t>& GetVec()
    {
        return seq;
    }
private:
    void PressReleaseKey(uint16_t scancode, bool press = true)
    {
#ifdef _WIN32
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = scancode;
        input.ki.dwFlags = (press ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_SCANCODE;
        if((scancode & 0xFF00) == 0xE000)
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        SendInput(1, &input, sizeof(input));
#else

#endif
    }

    std::vector<uint16_t> seq; /* scan codes to press and release*/
    static inline const char* name = "SEQUENCE";
};

template<class> inline constexpr bool always_false_v = false;

class KeyDelay final : public IKey
{
public:
    KeyDelay(uint32_t delay_) : delay(delay_)
    {
        
    }    
    KeyDelay(uint32_t start_, uint32_t end_) : delay(std::array<uint32_t, 2>{start_, end_})
    {
        //delay = ;
    }
    KeyDelay(std::string&& str);
    KeyDelay(const KeyDelay& from)
    {
        delay = from.delay;
    }
    virtual ~KeyDelay() { }
    KeyDelay* Clone() override
    {
        return new KeyDelay(*this);
    }

    void Execute() override
    {
        std::visit([](auto&& arg) 
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, uint32_t>)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(arg));
                }
                else if constexpr(std::is_same_v<T, std::array<uint32_t, 2>>)
                {
                    static boost::mt19937 gen;
                    boost::uniform_int<> dist(arg[0], arg[1]);
                    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(gen, dist);
                    int ret = die();
                    std::this_thread::sleep_for(std::chrono::milliseconds(ret));
                }
                else
                    static_assert(always_false_v<T>, "bad visitor!");
            }, delay);
    }
    std::string GenerateText(bool is_ini_format) override;
    const char* GetName() override { return std::holds_alternative<uint32_t>(delay) ? name_delay : name_random; }
    
    std::variant<uint32_t, std::array<uint32_t, 2>>& GetDelay()
    {
        return delay;
    }

private:
    std::variant<uint32_t, std::array<uint32_t, 2>> delay;
    static inline const char* name_delay = "DELAY";
    static inline const char* name_random = "DELAY RANDOM";
};

class MouseMovement final : public IKey
{
public:
    MouseMovement(LPPOINT* pos_)
    {
        memcpy(&m_pos, pos_, sizeof(m_pos));
    }
    MouseMovement(const MouseMovement& from)
    {
        memcpy(&m_pos, &from.m_pos, sizeof(m_pos));
    }
    MouseMovement(std::string&& str);
    virtual ~MouseMovement() { }
    MouseMovement* Clone() override
    {
        return new MouseMovement(*this);
    }

    void Execute() override
    {
#ifdef _WIN32
        POINT to_screen;
        HWND hwnd = GetForegroundWindow();
        memcpy(&to_screen, &m_pos, sizeof(to_screen));
        ClientToScreen(hwnd, &to_screen);
        ShowCursor(FALSE);
        SetCursorPos(to_screen.x, to_screen.y);
        ShowCursor(TRUE);
#else
        system(fmt::format("xte 'mousemove {} {}'", 0, 0).c_str());
#endif
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_MOVE[{},{}]", m_pos.x, m_pos.y) : fmt::format("{},{}", m_pos.x, m_pos.y);
        return ret;
    }
    const char* GetName() override { return name; }

    POINT& GetPos()
    {
        return m_pos;
    }

private:
    POINT m_pos = {};
    static inline const char* name = "MOUSE MOVE";
};

class MouseInterpolate final : public IKey
{
public:
    MouseInterpolate(LPPOINT* pos_)
    {
        memcpy(&m_pos, pos_, sizeof(m_pos));
    }
    MouseInterpolate(const MouseInterpolate& from)
    {
        memcpy(&m_pos, &from.m_pos, sizeof(m_pos));
    }
    MouseInterpolate(std::string&& str);
    virtual ~MouseInterpolate() { }
    MouseInterpolate* Clone() override
    {
        return new MouseInterpolate(*this);
    }

    void Execute() override
    {
#ifdef _WIN32
        /* TODO: interpolation from start pos to destination */
        POINT to_screen;
        HWND hwnd = GetForegroundWindow();
        memcpy(&to_screen, &m_pos, sizeof(to_screen));
        ClientToScreen(hwnd, &to_screen);
        ShowCursor(FALSE);
        SetCursorPos(to_screen.x, to_screen.y);
        ShowCursor(TRUE);
#else
        system(fmt::format("xte 'mousemove {} {}'", 0, 0).c_str());
#endif
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_INTERPOLATE[{},{}]", m_pos.x, m_pos.y) : fmt::format("{},{}", m_pos.x, m_pos.y);
        return ret;
    }
    const char* GetName() override { return name; }

    POINT& GetPos()
    {
        return m_pos;
    }

private:
    POINT m_pos = {};
    static inline const char* name = "MOUSE INTERPOLATE";
};

class MousePress final : public IKey
{
public:
    MousePress(uint16_t key_) : key(key_)
    {}
    MousePress(const MousePress& from)
    {
        key = from.key;
    }
    MousePress(const std::string&& str);
    virtual ~MousePress() { }
    MousePress* Clone() override
    {
        return new MousePress(*this);
    }

    void Execute() override
    {
        PressMouse(key);
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string text;
#ifdef _WIN32
        switch(key)
        {
        case MOUSEEVENTF_LEFTDOWN:
            text = "LEFT";
            break;
        case MOUSEEVENTF_RIGHTDOWN:
            text = "RIGHT";
            break;
        case MOUSEEVENTF_MIDDLEDOWN:
            text = "MIDDLE";
            break;
        default:
            assert(0);
        }
#endif
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_PRESS[{}]", text) : text;
        return ret;
    }
    const char* GetName() override { return name; }

    uint16_t GetKey()
    {
        return key;
    }
private:
    void PressMouse(uint16_t mouse_button)
    {
#ifdef _WIN32
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = mouse_button;
        SendInput(1, &input, sizeof(input));
#else

#endif
    }

    uint16_t key;
    static inline const char* name = "MOUSE PRESS";
};

class MouseRelease final : public IKey
{
public:
    MouseRelease(uint16_t key_) : key(key_)
    {}
    MouseRelease(const MouseRelease& from)
    {
        key = from.key;
    }
    MouseRelease(const std::string&& str);
    virtual ~MouseRelease() { }
    MouseRelease* Clone() override
    {
        return new MouseRelease(*this);
    }

    void Execute() override
    {
        ReleaseMouse(key);
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string text;
#ifdef _WIN32
        switch(key)
        {
        case MOUSEEVENTF_LEFTDOWN:
            text = "LEFT";
            break;
        case MOUSEEVENTF_RIGHTDOWN:
            text = "RIGHT";
            break;
        case MOUSEEVENTF_MIDDLEDOWN:
            text = "MIDDLE";
            break;
        default:
            assert(0);
        }
#endif
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_RELEASE[{}]", text) : text;
        return ret;
    }
    const char* GetName() override { return name; }

    uint16_t GetKey()
    {
        return key;
    }
private:
    void ReleaseMouse(uint16_t mouse_button)
    {
#ifdef _WIN32
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = mouse_button << (uint16_t)1;
        SendInput(1, &input, sizeof(input));
#else

#endif
    }

    uint16_t key;
    static inline const char* name = "MOUSE RELEASE";
};

class MouseClick final : public IKey
{
public:
    MouseClick(uint16_t key_) : key(key_)
    {}
    MouseClick(const MouseClick& from)
    {
        key = from.key;
    }
    MouseClick(const std::string&& str);
    virtual ~MouseClick() { }
    MouseClick* Clone() override
    {
        return new MouseClick(*this);
    }

    void Execute() override
    {
        PressReleaseMouse(key);
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string text;
#ifdef _WIN32
        switch(key)
        {
            case MOUSEEVENTF_LEFTDOWN:
                text = "LEFT";
                break;
            case MOUSEEVENTF_RIGHTDOWN:
                text = "RIGHT";
                break;
            case MOUSEEVENTF_MIDDLEDOWN:
                text = "MIDDLE";
                break;
            default:
                assert(0);
        }
#endif
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_CLICK[{}]", text) : text;
        return ret;
    }
    const char* GetName() override { return name; }

    uint16_t GetKey()
    {
        return key;
    }
private:
    void PressReleaseMouse(uint16_t mouse_button)
    {
#ifdef _WIN32
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = mouse_button;
        SendInput(1, &input, sizeof(input));
        input.mi.dwFlags = mouse_button << (uint16_t)1;
        SendInput(1, &input, sizeof(input));
#else
        
#endif
    }

    uint16_t key;
    static inline const char* name = "MOUSE CLICK";
};

/* each given macro per-app get's a macro container */
class MacroAppProfile
{
public:
    MacroAppProfile() = default;
    MacroAppProfile(std::string&& name)
    {
        app_name = std::move(name);
    }

    // \brief Map of macros per code, [Key code] = macro list (KeyText, KeySequence, etc...)
    std::map<std::string, std::vector<std::unique_ptr<IKey>>> key_vec;

    // \brief Binding name - [Key code] = bind name text
    std::map<std::string, std::string> bind_name;

    // \brief Name of assigned application - like Visual Studio
    std::string app_name;
private:
};

class CustomMacro : public CSingleton < CustomMacro >
{
    friend class CSingleton < CustomMacro >;

public:
    CustomMacro() = default;
    ~CustomMacro() = default;

    template<typename T> void OnItemRecordingStarted(std::unique_ptr<T>&& val)
    {
        editing_macro->push_back(std::move(val));
        editing_item = editing_macro->back().get();
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
            frame->config_panel->keybrd_panel->UpdateDetailsTree();
    }  
   
    template<typename T> void OnItemRecordingComplete(std::unique_ptr<T>&& val)
    {
        editing_macro->push_back(std::move(val));
        editing_item = editing_macro->back().get();
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
            frame->config_panel->keybrd_panel->UpdateDetailsTree();

    }
    std::vector<std::unique_ptr<MacroAppProfile>>& GetMacros()
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

    const std::unordered_map<std::string, int>& GetHidScanCodeMap()
    {
        return scan_codes;
    }

    bool use_per_app_macro = true;
    bool advanced_key_binding = true;
    std::vector<std::unique_ptr<IKey>>* editing_macro = nullptr;
    IKey* editing_item = nullptr;
    void ProcessReceivedData(const char* data, unsigned int len);

private:
    friend class Settings;

    void PressKey(std::string key);

    std::vector<std::unique_ptr<MacroAppProfile>> macros;
    std::string pressed_keys;
    static const std::unordered_map<std::string, int> scan_codes;
    static const std::unordered_map<int, std::string> hid_scan_codes;
};