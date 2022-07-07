#pragma once

#include "utils/CSingleton.hpp"

class KeyInfo
{
public:
    KeyInfo(uint32_t scanCode_, uint32_t flags_, std::string&& str, bool is_down_) :
        scanCode(scanCode_), flags(flags_), key(std::move(str)), is_down(is_down_)
    {

    };

    KeyInfo(const KeyInfo&& from) noexcept
    {
        scanCode = from.scanCode;
        key = from.key;
        flags = from.flags;
        is_down = from.is_down;
    }
    /*
    KeyInfo(const KeyInfo& from)
    {
        scanCode = from.scanCode;
        flags = from.flags;
        key = from.key;
    }
    */

    bool operator<(const KeyInfo& rhs) const
    {
        return scanCode < rhs.scanCode/* && flags < rhs.flags*/;
    }
    bool operator==(const KeyInfo& rhs) const
    {
        return rhs.scanCode == scanCode/* && rhs.flags == flags*/; /* don't care about flags here */
    }

    KeyInfo& operator= (KeyInfo&&) noexcept { return *this; }
    uint32_t scanCode;
    std::string key;
    uint32_t flags;
    bool is_down; /* 1 = down (pressed), 1 = up (released) */
};

class MacroRecorder : public CSingleton < MacroRecorder >
{
    friend class CSingleton < MacroRecorder >;

public:
    void Init() { }

    MacroRecorder() = default;
    ~MacroRecorder() = default;

    void StartRecording(bool kbd, bool mouse);
    void StopRecording();
    bool IsRecordingKeyboard();
    bool IsRecordingMouse();
    void MarkMousePosition(LPPOINT* pos);
    std::string GetKeyFromScanCode(int scancode, uint32_t flags);

#ifdef _WIN32
    void OnKeyPressed(KBDLLHOOKSTRUCT* p);
    void OnKeyReleased(KBDLLHOOKSTRUCT* p);
#endif
private:
    void FinishTextMacro(std::string& out, bool clear);

    std::vector<KeyInfo> key_press;  /* TODO: consider using (smart?) pointers here */
    uint32_t last_key_down = std::numeric_limits<uint32_t>::max();

    uint8_t current_macro_type = 0xFF;
    uint8_t last_macro_type = 0xFF;
    bool is_caps = false;
    bool is_shift = false;

    typedef struct
    {
        int scancode;
        uint32_t flags;
        bool is_modifier_key;
    } KeyAndFlags_t;
    static const std::unordered_map<std::string, KeyAndFlags_t> scan_codes;
};
