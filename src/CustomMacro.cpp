#include "pch.hpp"

void KeyText::Execute()
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

std::string KeyText::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" KEY_TYPE[{}]", seq) : seq;
    return ret;
}

#ifdef _WIN32
void KeyText::TypeCharacter(uint16_t character)
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

KeyCombination::KeyCombination(std::string&& str)
{
    boost::erase_all(str, " ");
    boost::char_separator<char> sep("+");
    boost::tokenizer<boost::char_separator<char>> tok(str, sep);
    for(boost::tokenizer<boost::char_separator<char>>::iterator beg = tok.begin(); beg != tok.end(); ++beg)
    {
        std::string key_code = *beg;
        uint16_t key = CustomMacro::Get()->GetKeyScanCode(key_code);
        if(key == 0xFFFF) /* do not throw here! */
        {
            LOG(LogLevel::Error, "Invalid key found in settings.ini: {}", key_code);
        }
        seq.push_back(key);
    }
}

void KeyCombination::Execute()
{
    for(size_t i = 0; i < seq.size(); i++)
        PressReleaseKey(seq[i]);
    for(size_t i = 0; i < seq.size(); i++)
        PressReleaseKey(seq[i], false);
}

std::string KeyCombination::GenerateText(bool is_ini_format)
{
    std::string text;
    for(auto& i : seq)
    {
        text += CustomMacro::Get()->GetKeyStringFromScanCode(i) + "+";
    }
    if(text.back() == '+')
        text.pop_back();
    std::string ret = is_ini_format ? std::format(" KEY_SEQ[{}]", text) : text;
    return ret;
}

void KeyCombination::PressReleaseKey(uint16_t scancode, bool press)
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

KeyDelay::KeyDelay(std::string&& str)
{
    size_t separator_pos = str.find("-");
    if(separator_pos != std::string::npos)
    {
        boost::erase_all(str, " ");
        uint32_t delay_start = utils::stoi<uint32_t>(str);
        uint32_t delay_end = utils::stoi<uint32_t>(&str[separator_pos + 1]);
        delay = std::array<uint32_t, 2>{delay_start, delay_end};
    }
    else
    {
        delay = utils::stoi<uint32_t>(str);
    }
}

void KeyDelay::Execute()
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
                int ret = utils::random_mt(arg[0], arg[1]);
                std::this_thread::sleep_for(std::chrono::milliseconds(ret));
            }
            else
                static_assert(always_false_v<T>, "bad visitor!");
        }, delay);
}

std::string KeyDelay::GenerateText(bool is_ini_format)
{
    std::string ret;
    if(std::holds_alternative<uint32_t>(delay))
    {
        ret = is_ini_format ? std::format(" DELAY[{}]", std::get<uint32_t>(delay)) : boost::lexical_cast<std::string>(std::get<uint32_t>(delay));
    }
    else
    {
        std::array<uint32_t, 2> delays = std::get<std::array<uint32_t, 2>>(delay);
        ret = is_ini_format ? std::format(" DELAY[{}-{}]", delays[0], delays[1]) : boost::lexical_cast<std::string>(delays[0]) + "-" + boost::lexical_cast<std::string>(delays[1]);
    }
    return ret;
}

MouseMovement::MouseMovement(std::string&& str)
{
    size_t separator_pos = str.find(",");
    if(separator_pos != std::string::npos)
    {
        boost::erase_all(str, " ");
        m_pos.x = utils::stoi<decltype(m_pos.x)>(str);
        m_pos.y = utils::stoi<decltype(m_pos.y)>(&str[separator_pos + 1]);
    }
    else
        throw std::invalid_argument(std::format("Invalid mouse movement input: {}", str));
}

void MouseMovement::Execute()
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

std::string MouseMovement::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" MOUSE_MOVE[{},{}]", m_pos.x, m_pos.y) : std::format("{},{}", m_pos.x, m_pos.y);
    return ret;
}

MouseInterpolate::MouseInterpolate(std::string&& str)
{
    size_t separator_pos = str.find(",");
    if(separator_pos != std::string::npos)
    {
        boost::erase_all(str, " ");
        m_pos.x = utils::stoi<decltype(m_pos.x)>(str);
        m_pos.y = utils::stoi<decltype(m_pos.y)>(&str[separator_pos + 1]);
    }
    else
        throw std::invalid_argument(std::format("Invalid mouse interpolate input: {}", str));
}

void MouseInterpolate::Execute()
{
#ifdef _WIN32
    POINT to_screen;
    HWND hwnd = GetForegroundWindow();
    memcpy(&to_screen, &m_pos, sizeof(to_screen));
    ClientToScreen(hwnd, &to_screen);

    POINT curr_pos;
    GetCursorPos(&curr_pos);

    int steps = utils::random_mt(4000, 8000);
    for(int i = 0; i <= steps; i++)
    {
        int pos_x = std::round(std::lerp(static_cast<float>(curr_pos.x), static_cast<float>(to_screen.x), static_cast<float>(i) / static_cast<float>(steps)));
        int pos_y = std::round(std::lerp(static_cast<float>(curr_pos.y), static_cast<float>(to_screen.y), static_cast<float>(i) / static_cast<float>(steps)));
        ShowCursor(FALSE);
        SetCursorPos(pos_x, pos_y);
        ShowCursor(TRUE);
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
#else
    system(fmt::format("xte 'mousemove {} {}'", 0, 0).c_str());
#endif
}

std::string MouseInterpolate::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" MOUSE_INTERPOLATE[{},{}]", m_pos.x, m_pos.y) : std::format("{},{}", m_pos.x, m_pos.y);
    return ret;
}

MousePress::MousePress(const std::string&& str)
{
    uint16_t mouse_button = 0xFFFF;
#ifdef _WIN32
    if(str == "L" || str == "LEFT")
        mouse_button = MOUSEEVENTF_LEFTDOWN;
    if(str == "R" || str == "RIGHT")
        mouse_button = MOUSEEVENTF_RIGHTDOWN;
    if(str == "M" || str == "MIDDLE")
        mouse_button = MOUSEEVENTF_MIDDLEDOWN;
#else
    mouse_button = 0;
#endif
    if(mouse_button != 0xFFFF)
        key = mouse_button;
    else
        throw std::invalid_argument(std::format("Invalid mouse button input: {}", str));
}

void MousePress::Execute()
{
    PressMouse(key);
}

std::string MousePress::GenerateText(bool is_ini_format)
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
    std::string ret = is_ini_format ? std::format(" MOUSE_PRESS[{}]", text) : text;
    return ret;
}

void MousePress::PressMouse(uint16_t mouse_button)
{
#ifdef _WIN32
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = mouse_button;
    SendInput(1, &input, sizeof(input));
#else

#endif
}

MouseRelease::MouseRelease(const std::string&& str)
{
    uint16_t mouse_button = 0xFFFF;
#ifdef _WIN32
    if(str == "L" || str == "LEFT")
        mouse_button = MOUSEEVENTF_LEFTDOWN;
    if(str == "R" || str == "RIGHT")
        mouse_button = MOUSEEVENTF_RIGHTDOWN;
    if(str == "M" || str == "MIDDLE")
        mouse_button = MOUSEEVENTF_MIDDLEDOWN;
#else
    mouse_button = 0;
#endif
    if(mouse_button != 0xFFFF)
        key = mouse_button;
    else
        throw std::invalid_argument(std::format("Invalid mouse button input: {}", str));
}

void MouseRelease::Execute()
{
    ReleaseMouse(key);
}

std::string MouseRelease::GenerateText(bool is_ini_format)
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
    std::string ret = is_ini_format ? std::format(" MOUSE_RELEASE[{}]", text) : text;
    return ret;
}

void MouseRelease::ReleaseMouse(uint16_t mouse_button)
{
#ifdef _WIN32
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = mouse_button << (uint16_t)1;
    SendInput(1, &input, sizeof(input));
#else

#endif
}

MouseClick::MouseClick(const std::string&& str)
{
    uint16_t mouse_button = 0xFFFF;
#ifdef _WIN32
    if(str == "L" || str == "LEFT")
        mouse_button = MOUSEEVENTF_LEFTDOWN;
    if(str == "R" || str == "RIGHT")
        mouse_button = MOUSEEVENTF_RIGHTDOWN;
    if(str == "M" || str == "MIDDLE")
        mouse_button = MOUSEEVENTF_MIDDLEDOWN;
#else
    mouse_button = 0;
#endif
    if(mouse_button != 0xFFFF)
        key = mouse_button;
    else
        throw std::invalid_argument(std::format("Invalid mouse button input: {}", str));
}

void MouseClick::Execute()
{
    PressReleaseMouse(key);
}

std::string MouseClick::GenerateText(bool is_ini_format)
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
    std::string ret = is_ini_format ? std::format(" MOUSE_CLICK[{}]", text) : text;
    return ret;
}

void MouseClick::PressReleaseMouse(uint16_t mouse_button)
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

void BashCommand::Execute()
{
#ifdef _WIN32
    std::wstring param(cmd.begin(), cmd.end());
    std::wstring command = L"/k " + param;
    ShellExecuteW(NULL, L"open", L"cmd", command.c_str(), NULL, SW_NORMAL);
#else

#endif
}

std::string BashCommand::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" BASH[{}]", cmd) : cmd;
    return ret;
}

void CommandExecute::Execute()
{
#ifdef _WIN32
    std::wstring param(cmd.begin(), cmd.end());
    std::wstring command = L"/k " + param;
    utils::ExecuteCmdWithoutWindow(command.c_str());
#else

#endif
}

std::string CommandExecute::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" CMD[{}]", cmd) : cmd;
    return ret;
}

void CustomMacro::PressKey(std::string key)
{
    std::scoped_lock lock(executor_mtx);
    if(PrintScreenSaver::Get()->screenshot_key == pressed_keys)
    {
        PrintScreenSaver::Get()->SaveScreenshot();
        return;
    }
    if(PathSeparator::Get()->replace_key == pressed_keys)
    {
        PathSeparator::Get()->ReplaceClipboard();
        return;
    }
    if(SymlinkCreator::Get()->HandleKeypress(pressed_keys))
        return;

    auto ExecuteGlobalMacro = [this]()
    {
        const auto it = macros[0]->key_vec.find(pressed_keys);
        if(it != macros[0]->key_vec.end())
        {
            for(const auto& i : it->second)
            {
                i->Execute();
            }
        }
    };

    if(use_per_app_macro)
    {
#ifdef _WIN32
        HWND foreground = GetForegroundWindow();
        if(foreground)
        {
            char window_title[256];
            GetWindowTextA(foreground, window_title, sizeof(window_title));
            bool app_found = false;
            for(auto& m : macros)
            {
                if(boost::algorithm::contains(window_title, m->app_name) && m->app_name.length() > 2)
                {
                    const auto it = m->key_vec.find(pressed_keys);
                    if(it != m->key_vec.end())
                    {
                        for(const auto& i : it->second)
                        {
                            i->Execute();
                        }
                    }
                    app_found = true;
                    break;
                }
            }
            if(!app_found)
                ExecuteGlobalMacro();
        }
        else
        {
            ExecuteGlobalMacro();
        }
#else
        ExecuteGlobalMacro();
#endif
    }
    else
    {
        ExecuteGlobalMacro();
    }
}

void CustomMacro::ProcessReceivedData(const char* data, unsigned int len)
{
    if(len != sizeof(KeyData_t))
        return;
    KeyData_t* k = (KeyData_t*)data;
    uint16_t crc = utils::crc16_modbus((void*)data, len - 2);
    if(!strncmp(data, "reset", 5) && crc == 0x6bd8) /* in case of suddenly reset of STM32 */
    {
        pressed_keys.clear();
    }

    if(k->crc == crc)
    {
        static const char all_released[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        if(!memcmp(&data[1], all_released, sizeof(all_released)))
        {
            pressed_keys.clear();
        }
        else
        {
            if(!pressed_keys.empty() && pressed_keys[pressed_keys.length() - 1] != '+')
                pressed_keys += '+';

            if(advanced_key_binding)
            {
                static const char no_keys[6] = { 0, 0, 0, 0, 0, 0 };
                if(!memcmp(k->keys, no_keys, sizeof(k->keys)))  /* stop the execution when only control keys was pressed */
                {
                    return;
                }
            }

            if(k->lshift)
                pressed_keys += "LSHIFT";
            else if(k->lalt)
                pressed_keys += "LALT";
            else if(k->lgui)
                pressed_keys += "LGUI";
            else if(k->lctrl)
                pressed_keys += "LCTRL";
            if(k->rshift)
                pressed_keys += "RSHIFT";
            else if(k->ralt)
                pressed_keys += "RALT";
            else if(k->rgui)
                pressed_keys += "RGUI";
            else if(k->rctrl)
                pressed_keys += "RCTRL";

            for(int i = 0; i != 1; i++) // TODO: sizeof(k->keys) / sizeof(k->keys[0])
            {   /* only supporting 1 addon key (for now?), it should be more than enough - possible to bind more than 700 macros - PER APPLICATION! */
                auto key_str = hid_scan_codes.find(k->keys[i]);
                if(key_str != hid_scan_codes.end())
                {
                    if(!pressed_keys.empty() && pressed_keys[pressed_keys.length() - 1] != '+')
                        pressed_keys += '+';

                    pressed_keys += key_str->second;
                }
            }
            PressKey(pressed_keys);
        }
    }
}

const std::unordered_map<std::string, int> CustomMacro::scan_codes = 
{
            {"LCTRL",       0x1D},
            {"RCTRL",       0xE01D},
            {"LALT",        0x38},
            {"RALT",        0xE038},
            {"LSHIFT",      0x2A},
            {"RSHIFT",      0x36},
            {"BACKSPACE",   0xE},
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
            {"AFTERBURNER", 0xFFFE},
};

const std::unordered_map<int, std::string> CustomMacro::hid_scan_codes =
{       
        {0x04, "A"},
        {0x05, "B"},
        {0x06, "C"},
        {0x07, "D"},
        {0x08, "E"},
        {0x09, "F"},
        {0x0A, "G"},
        {0x0B, "H"},
        {0x0C, "I"},
        {0x0D, "J"},
        {0x0E, "K"},
        {0x0F, "L"},
        {0x10, "M"},
        {0x11, "N"},
        {0x12, "O"},
        {0x13, "P"},
        {0x14, "Q"},
        {0x15, "R"},
        {0x16, "S"},
        {0x17, "T"},
        {0x18, "U"},
        {0x19, "V"},
        {0x1A, "W"},
        {0x1B, "X"},
        {0x1C, "Y"},
        {0x1D, "Z"},
        {0x1E, "1"},
        {0x1F, "2"},
        {0x20, "3"},
        {0x21, "4"},
        {0x22, "5"},
        {0x23, "6"},
        {0x24, "7"},
        {0x25, "8"},
        {0x26, "9"},
        {0x27, "0"},
        {0x2A, "NUM_BACKSPACE"},
        {0x3A, "F1"},
        {0x3B, "F2"},
        {0x3C, "F3"},
        {0x3D, "F4"},
        {0x3E, "F5"},
        {0x3F, "F6"},
        {0x40, "F7"},
        {0x41, "F8"},
        {0x42, "F9"},
        {0x43, "F10"},
        {0x44, "F11"},
        {0x45, "F12"},
        {0x46, "PRINT"},
        {0x47, "SCROLL"},
        {0x48, "PUASE"},
        {0x49, "INSERT"},
        {0x4A, "HOME"},
        {0x4B, "PAGEUP"},
        {0x4C, "DELETE"},
        {0x4D, "END"},
        {0x4E, "PAGEDOWN"},
        {0x4F, "RIGHT"},
        {0x50, "LEFT"},
        {0x51, "DOWN"},
        {0x52, "UP"},
        {0x53, "NUM_LOCK"},
        {0x54, "NUM_DIV"},
        {0x55, "NUM_MUL"},
        {0x56, "NUM_MINUS"},
        {0x57, "NUM_PLUS"},
        {0x58, "ENTER"},
        {0x59, "NUM_1"},
        {0x5A, "NUM_2"},
        {0x5B, "NUM_3"},
        {0x5C, "NUM_4"},
        {0x5D, "NUM_5"},
        {0x5E, "NUM_6"},
        {0x5F, "NUM_7"},
        {0x60, "NUM_8"},
        {0x61, "NUM_9"},
        {0x62, "NUM_0"},
        {0x63, "NUM_DEL"},
        {0xF0, "LCTRL"},
        {0xF1, "LSHIFT"},
        {0xF2, "LALT"},
        {0xF3, "LGUI"},
        {0xF4, "RCTRL"},
        {0xF5, "RSHIFT"},
        {0xF6, "RALT"},
        {0xF7, "RGUI"}
};