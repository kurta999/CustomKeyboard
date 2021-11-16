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
#include <boost/tokenizer.hpp>

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
    BIND_NAME, KEY_SEQ, KEY_TYPE, DELAY, MOUSE_MOVE, MOUSE_CLICK, MAX
};

class KeyClass
{
public:
    KeyClass() {}
    virtual ~KeyClass() {}
    KeyClass(const KeyClass&) {}
    virtual void DoWrite() = 0;
    virtual std::string GenerateText(bool is_ini_format) = 0;
    virtual const char* GetName() = 0;
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
#ifdef _WIN32
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
#else
    #error "This function is only implemented for Window"
#endif
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
    std::string seq; /* characters to press and release*/
    static inline const char* name = "TEXT";
};

class KeyCombination final : public KeyClass
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

    void DoWrite() override
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
    std::vector<uint16_t> seq; /* scan codes to press and release*/
    static inline const char* name = "SEQUENCE";
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

    void DoWrite() override
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
                    static_assert(always_false<T>::value, "bad visitor!");
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
    MouseMovement(std::string&& str);
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

    std::string GenerateText(bool is_ini_format) override
    {
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_MOVE[{},{}]",  pos.x, pos.y) : fmt::format("{},{}", pos.x, pos.y);
        return ret;
    }
    const char* GetName() override { return name; }

    POINT& GetPos()
    {
        return pos;
    }

private:
    POINT pos = {};
    static inline const char* name = "MOUSE MOVE";
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
    MouseClick(const std::string&& str);
    virtual ~MouseClick() { }
    MouseClick* Clone() override
    {
        return new MouseClick(*this);
    }

    void DoWrite() override
    {
        PressReleaseMouse(key);
    }

    std::string GenerateText(bool is_ini_format) override
    {
        std::string text;
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
        std::string&& ret = is_ini_format ? fmt::format(" MOUSE_CLICK[{}]", text) : text;
        return ret;
    }
    const char* GetName() override { return name; }

    uint16_t GetKey()
    {
        return key;
    }
private:
    uint16_t key;
    static inline const char* name = "MOUSE CLICK";
};

/* each given macro per-app get's a macro container */
class MacroContainer
{
public:
    MacroContainer() = default;
    MacroContainer(std::string&& _name)
    {
        name = std::move(_name);
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
    CustomMacro();
    ~CustomMacro();
    void Init();

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

    const std::unordered_map<std::string, int>& GetHidScanCodeMap()
    {
        return scan_codes;
    }

    bool is_enabled = true;
    uint16_t com_port = 5;
    bool use_per_app_macro = true;
    bool advanced_key_binding = true;
    std::vector<std::unique_ptr<KeyClass>>* editing_macro = nullptr;
    KeyClass* editing_item = nullptr;

private:
    friend class Settings;

    void DestroyWorkingThread();
    void PressKey(std::string key);
    void UartDataReceived(const char* data, unsigned int len);
    void UartReceiveThread(std::atomic<bool>& to_exit, std::condition_variable& cv, std::mutex &m);

    std::vector<std::unique_ptr<MacroContainer>> macros;
    std::string pressed_keys;
    std::thread *t = nullptr;
    std::atomic<bool> to_exit = false;
    std::condition_variable cv;
    std::mutex m;
    static const std::unordered_map<std::string, int> scan_codes;
    static const std::unordered_map<int, std::string> hid_scan_codes;
};