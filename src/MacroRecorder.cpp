#include "pch.h"

static HHOOK hHook_keyboard = nullptr;
static HHOOK hHook_mouse = nullptr;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        switch(wParam)
        {
            case WM_SYSKEYDOWN:  /* for ALT */
            case WM_KEYDOWN: /* for other keys */
            {
                MacroRecorder::Get()->OnKeyPressed(p);
                break;
            }
            case WM_SYSKEYUP:  /* for ALT */
            case WM_KEYUP:  /* for other keys */
            {
                MacroRecorder::Get()->OnKeyReleased(p);
                break;
            }
        }
    }
    return CallNextHookEx(hHook_keyboard, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;
    uint16_t key = 0xFFFF;
    switch(wParam)
    {
        case WM_LBUTTONDOWN:
            key = MOUSEEVENTF_LEFTDOWN;
            break;
        case WM_RBUTTONDOWN:
            key = MOUSEEVENTF_RIGHTDOWN;
            break;
        case WM_MBUTTONDOWN:
            key = MOUSEEVENTF_MIDDLEDOWN;
            break;
    }
    if(key != 0xFFFF)
        CustomMacro::Get()->OnItemRecordingComplete(std::make_unique<MouseClick>(key));
    return CallNextHookEx(0, nCode, wParam, lParam);
}

void MacroRecorder::OnKeyPressed(KBDLLHOOKSTRUCT* p)
{
    if(last_key_down != p->scanCode)  /* avoid adding same key more times when it's being hold down */
    {
        std::string key = MacroRecorder::Get()->GetKeyFromScanCode(p->scanCode, is_shift ? LLKHF_EXTENDED : 0);
        if(key == "LSHIFT" || key == "RSHIFT")
            is_shift = true;
        is_caps = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

        if(key.find("CTRL") != std::string::npos || key.find("ALT") != std::string::npos || key.find("TAB") != std::string::npos ||
            key.find("ESC") != std::string::npos || key.find("BACKSPACE") != std::string::npos || key.find("ENTER") != std::string::npos ||
            key.find("AAA") != std::string::npos) /* sequence */
        {
            if(current_macro_type == KEY_TYPE)
            {
                std::string text;
                bool is_capital = false;
                for(auto& i : key_press)
                {
                    if(i.is_down == 1 && (i.key == "LSHIFT" || i.key == "RSHIFT"))
                    {
                        is_capital = true;
                        continue;
                    }

                    if(i.is_down == 0 && (i.key == "LSHIFT" || i.key == "RSHIFT") && is_capital)
                    {
                        is_capital = false;
                        continue;
                    }
                    if(i.is_down == 1)
                    {
                        std::string chr = MacroRecorder::Get()->GetKeyFromScanCode(i.scanCode, is_capital ? LLKHF_EXTENDED : 0);
                        utils::MadeTextFromKeys(chr);

                        if(chr.length() <= 1)
                            text += chr;
                        else
                            DBG("NOT TEXT ERROR: %s\n", chr.c_str());
                    }
                }
                DBG("text final: %s\n", text.c_str());
                CustomMacro::Get()->OnItemRecordingComplete(std::make_unique<KeyText>(std::move(text)));
                current_macro_type = 0xFF;
                key_press.clear();
            }
            current_macro_type = MacroTypes::KEY_SEQ;
        }

        if((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) /* text */
        {
            if(current_macro_type == 0xFF)
                current_macro_type = MacroTypes::KEY_TYPE;
        }

        DBG("key down %s, %d, e: %d, caps: %d, shift: %d\n", key.c_str(), p->scanCode, (p->flags & LLKHF_EXTENDED) != 0, is_caps, is_shift);
        key_press.push_back({ p->scanCode, p->flags, std::move(key), true });

        last_key_down = p->scanCode;
    }
}

void MacroRecorder::OnKeyReleased(KBDLLHOOKSTRUCT* p)
{
    std::string key = CustomMacro::Get()->GetKeyStringFromScanCode(p->scanCode); /* todo: replace with GetKeyFromScanCode */
    if(key == "LSHIFT" || key == "RSHIFT")
        is_shift = false;

    last_key_down = std::numeric_limits<DWORD>::max();
    key_press.push_back({ p->scanCode, p->flags, std::move(key), false });
    DBG("key up %s\n", key.c_str());

    if(current_macro_type == MacroTypes::KEY_SEQ)
    {
        int down = std::count_if(key_press.begin(), key_press.end(), [](KeyInfo& x) { return x.is_down == 1; });
        int up = std::count_if(key_press.begin(), key_press.end(), [](KeyInfo& x) { return x.is_down == 0; });
        if(down == up)
        {
            DBG("SEQ keys released\n");
            std::vector<uint16_t> keys_seq;
            for(auto& i : key_press)
            {
                if(i.is_down == 0) continue;
                keys_seq.push_back(i.scanCode);
                DBG("%s ", i.key.c_str());
            }
            CustomMacro::Get()->OnItemRecordingComplete(std::make_unique<KeyCombination>(std::move(keys_seq)));
            DBG("\n");
            key_press.clear();
            current_macro_type = 0xFF;
        }
        else  /* just a simple sanity check - shouldn't happen */
        {
            DBG("key count mismatch: %d, %d", down, up);
            key_press.clear();
            current_macro_type = 0xFF;
        }
    }
}

void MacroRecorder::StartRecording(bool kbd, bool mouse)
{
    StopRecording();
    if(kbd)
        hHook_keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, &LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);
    if(mouse)
	    hHook_mouse = SetWindowsHookEx(WH_MOUSE_LL, &LowLevelMouseProc, GetModuleHandle(NULL), NULL);
}

void MacroRecorder::StopRecording()
{
    if(hHook_keyboard)
    {
        UnhookWindowsHookEx(hHook_keyboard);
        hHook_keyboard = nullptr;
    }
    if(hHook_mouse)
    {
        UnhookWindowsHookEx(hHook_mouse);
        hHook_mouse = nullptr;
    }
}

bool MacroRecorder::IsRecordingKeyboard()
{
    return hHook_keyboard != nullptr;
}

bool MacroRecorder::IsRecordingMouse()
{
    return hHook_mouse != nullptr;
}

void MacroRecorder::MarkMousePosition(LPPOINT* pos)
{
    if(hHook_mouse != nullptr)
    {
        CustomMacro::Get()->OnItemRecordingComplete(std::make_unique<MouseMovement>(pos));
    }
}

std::string MacroRecorder::GetKeyFromScanCode(int scancode, uint32_t flags)
{
    std::string ret = "INVALID";
    for(auto& i : scan_codes)
    {
        if(i.second.scancode == scancode)
        {
            if(flags != 0)
            {
                if((i.second.flags & flags) != 0)
                {
                    ret = i.first;
                    break;
                }
            }
            else
            {
                if(i.second.flags == 0)
                {
                    ret = i.first;
                    break;
                }
            }
            
        }
    }
    return ret;
}

const std::unordered_map<std::string, MacroRecorder::KeyAndFlags_t> MacroRecorder::scan_codes =
{
            {"LCTRL",       {0x1D, 0}},
            {"RCTRL",       {0x1D, LLKHF_EXTENDED}},
            {"LALT",        {0x38, 0}},
            {"RALT",        {0x38, LLKHF_EXTENDED}},
            {"LSHIFT",      {0x2A, 0}},
            {"RSHIFT",      {0x36, 0}},
            {"BACKSPACE",   {0xE, 0}},
            {"TAB",         {0x0F, 0}},
            {"ENTER",       {0x1C, 0}},
            {"ESC",         {0x01, 0}},
            {"SPACE",       {0x39, 0}},
            {"PAGEUP",      {0xE049, 0}},
            {"PAGEDOWN",    {0xE051, 0}},
            {"END",         {0xE04F, 0}},
            {"HOME",        {0xE047, 0}},
            {"PRINT",       {0x0, 0}}, // TODO
            {"INSERT",      {0xE052, 0}},
            {"DELETE",      {0xE053, 0}},
            {"1",           {0x4F, 0}}, /* num keys */
            {"2",           {0x50, 0}},
            {"3",           {0x51, 0}},
            {"4",           {0x4B, 0}},
            {"5",           {0x4C, 0}},
            {"6",           {0x4D, 0}},
            {"7",           {0x47, 0}},
            {"8",           {0x48, 0}},
            {"9",           {0x49, 0}},
            {"0",           {0x52, 0}},
            {"*",           {0x37, 0}},
            {",",           {0x53, 0}},
            {"+",           {0x4E, 0}},
            {"-",           {0x4A, 0}},
            {"/",           {53, LLKHF_EXTENDED}},
            {"a",           {0x1E, 0}},
            {"b",           {0x30, 0}},
            {"c",           {0x2E, 0}},
            {"d",           {0x20, 0}},
            {"e",           {0x12, 0}},
            {"f",           {0x21, 0}},
            {"g",           {0x22, 0}},
            {"h",           {0x23, 0}},
            {"i",           {0x17, 0}},
            {"j",           {0x24, 0}},
            {"k",           {0x25, 0}},
            {"l",           {0x26, 0}},
            {"m",           {0x32, 0}},
            {"n",           {0x31, 0}},
            {"o",           {0x18, 0}},
            {"p",           {0x19, 0}},
            {"q",           {0x10, 0}},
            {"r",           {0x13, 0}},
            {"s",           {0x1F, 0}},
            {"t",           {0x14, 0}},
            {"u",           {0x16, 0}},
            {"v",           {0x2F, 0}},
            {"w",           {0x11, 0}},
            {"x",           {0x2D, 0}},
            {"y",           {0x15, 0}},
            {"z",           {0x2C, 0}}, 
            {"A",           {0x1E, LLKHF_EXTENDED}},
            {"B",           {0x30, LLKHF_EXTENDED}},
            {"C",           {0x2E, LLKHF_EXTENDED}},
            {"D",           {0x20, LLKHF_EXTENDED}},
            {"E",           {0x12, LLKHF_EXTENDED}},
            {"F",           {0x21, LLKHF_EXTENDED}},
            {"G",           {0x22, LLKHF_EXTENDED}},
            {"H",           {0x23, LLKHF_EXTENDED}},
            {"I",           {0x17, LLKHF_EXTENDED}},
            {"J",           {0x24, LLKHF_EXTENDED}},
            {"K",           {0x25, LLKHF_EXTENDED}},
            {"L",           {0x26, LLKHF_EXTENDED}},
            {"M",           {0x32, LLKHF_EXTENDED}},
            {"N",           {0x31, LLKHF_EXTENDED}},
            {"O",           {0x18, LLKHF_EXTENDED}},
            {"P",           {0x19, LLKHF_EXTENDED}},
            {"Q",           {0x10, LLKHF_EXTENDED}},
            {"R",           {0x13, LLKHF_EXTENDED}},
            {"S",           {0x1F, LLKHF_EXTENDED}},
            {"T",           {0x14, LLKHF_EXTENDED}},
            {"U",           {0x16, LLKHF_EXTENDED}},
            {"V",           {0x2F, LLKHF_EXTENDED}},
            {"W",           {0x11, LLKHF_EXTENDED}},
            {"X",           {0x2D, LLKHF_EXTENDED}},
            {"Y",           {0x15, LLKHF_EXTENDED}},
            {"Z",           {0x2C, LLKHF_EXTENDED}},
            {"1",           {0x02, 0}},
            {"2",           {0x03, 0}},
            {"3",           {4, 0}},
            {"4",           {5, 0}},
            {"5",           {0x6, 0}},
            {"6",           {0x7, 0}},
            {"7",           {0x8, 0}},
            {"8",           {0x9, 0}},
            {"9",           {0xA, 0}},
            {"0",           {41, 0}},
            {"F1",          {0x3B, 0}},
            {"F2",          {0x3C, 0}},
            {"F3",          {0x3D, 0}},
            {"F4",          {0x3E, 0}},
            {"F5",          {0x3F, 0}},
            {"F6",          {0x40, 0}},
            {"F7",          {0x41, 0}},
            {"F8",          {0x42, 0}},
            {"F9",          {0x43, 0}},
            {"F10",         {0x44, 0}},
            {"F11",         {0x59, 0}},
            {"F12",         {0x58, 0}},
            {"LEFT",        {75, LLKHF_EXTENDED}},
            {"RIGHT",       {77, LLKHF_EXTENDED}},
            {"UP",          {72, LLKHF_EXTENDED}},
            {"DOWN",        {80, LLKHF_EXTENDED}},
            {"CAPSLOCK",    {58, 0}},
            {"LWIN",        {91, 0}},
            {"RWIN",        {92, 0}},
            {"INFO",        {93, 0}},
            {"í",           {86, 0}},
            {",",           {51, 0}},
            {".",           {52, 0}},
            {"é",           {39, 0}},
            {"á",           {40, 0}},
            {"û",           {43, 0}},
            {"õ",           {26, 0}},
            {"ú",           {27, 0}},
            {"ö",           {11, 0}},
            {"ó",           {13, 0}}
};

/* bool IsText(const std::string& key)
{
    return (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') || key == ' ';
} */