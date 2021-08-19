#pragma once

#include "utils/CSingleton.h"

class KeyInfo
{
public:
    KeyInfo(DWORD scanCode_, DWORD flags_, std::string&& str, bool is_down_) :
        scanCode(scanCode_), flags(flags_), key(std::move(str)), is_down(is_down_)
    {

    };

    KeyInfo(const KeyInfo&& from)
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

    KeyInfo& operator= (KeyInfo&&) { return *this; }
    DWORD scanCode;
    std::string key;
    DWORD flags;
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

    void OnKeyPressed(KBDLLHOOKSTRUCT* p);
    void OnKeyReleased(KBDLLHOOKSTRUCT* p);
private:
    std::vector<KeyInfo> key_press;  /* TODO: consider using (smart?) pointers here */
    DWORD last_key_down = std::numeric_limits<DWORD>::max();

    uint8_t current_macro_type = 0xFF;
    bool is_caps = false;
    bool is_shift = false;

    typedef struct
    {
        int scancode;
        uint32_t flags;
    } KeyAndFlags_t;
    static const std::unordered_map<std::string, KeyAndFlags_t> scan_codes;
};