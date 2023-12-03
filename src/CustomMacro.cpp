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
    if(!text.empty() && text.back() == '+')
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
                static_assert(always_false_v<T>, "KeyDelay::Execute Bad visitor!");
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

    int steps = utils::random_mt(1000, 3000);
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

void CommandXml::Execute()
{
    std::vector<std::string> params;
    boost::split(params, cmd, boost::is_any_of("+"));

    std::unique_ptr<CmdExecutor>& cmd_executor = wxGetApp().cmd_executor;
    if(params.size() == 2)
        cmd_executor->ExecuteByName(params[0], params[1]);
    else
    {
        LOG(LogLevel::Warning, "Invalid input: {}", cmd);
    }
}

std::string CommandXml::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" CMD_XML[{}]", cmd) : cmd;
    return ret;
}

KeyBringAppToForeground::KeyBringAppToForeground(const std::string&& str)
{
    std::vector<std::string> params;
    boost::split(params, str, boost::is_any_of(","));
    if(params.size() == 2)
    {
        app = std::move(params[0]);
        title = std::move(params[1]);
    }
    else
    {
        LOG(LogLevel::Warning, "Invalid input: {}", str);
    }
}

void KeyBringAppToForeground::Execute()
{
    ImageRecognition::BringWindowToForegroundByName(app, title);
}

std::string KeyBringAppToForeground::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" CMD_FG[{},{}]", app, title) : std::format("{},{}", app, title);
    return ret;
}

KeyFindImageOnScreen::KeyFindImageOnScreen(const std::string&& str)
{
    std::vector<std::string> params;
    boost::split(params, str, boost::is_any_of(","));
    if(params.size() == 3)
    {
        image_path = std::move(params[0]);
        try
        {
            offset.x = std::stoi(params[1]);
            offset.y = std::stoi(params[2]);
        }
        catch(const std::exception& e)
        {
            LOG(LogLevel::Warning, "Invalid input, exception: {} - {}", str, e.what());
        }
    }
    else
    {
        LOG(LogLevel::Warning, "Invalid input: {}", str);
    }
}

void KeyFindImageOnScreen::Execute()
{
    int x = 0, y = 0;
    bool ret = ImageRecognition::FindImageOnScreen(image_path.generic_string(), x, y);
    if(ret)
    {
        POINT pos;
        pos.x = x;
        pos.y = y;
        ImageRecognition::MoveCursorAndClick(pos);
    }
    else
    {
        LOG(LogLevel::Verbose, "Image isn't found on screen");
    }
}

std::string KeyFindImageOnScreen::GenerateText(bool is_ini_format)
{
    std::string ret = is_ini_format ? std::format(" CMD_IMG[{},{},{}]", image_path.generic_string(), offset.x, offset.y) : 
        std::format("{},{},{}", image_path.generic_string(), offset.x, offset.y);
    return ret;
}

void CustomMacro::ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroAppProfile>& c)
{
    constexpr std::underlying_type_t<MacroTypes> MAX_ITEMS = MacroTypes::MAX;
    constexpr const char* start_str_arr[MAX_ITEMS] = { "BIND_NAME[", "KEY_SEQ[", "KEY_TYPE[", "DELAY[", "MOUSE_MOVE[", "MOUSE_INTERPOLATE[",
        "MOUSE_PRESS[", "MOUSE_RELEASE", "MOUSE_CLICK[", "BASH[", "CMD[", "CMD_XML[", "CMD_FG[", "CMD_IMG["};
    constexpr const size_t start_str_arr_lens[MAX_ITEMS] = { std::char_traits<char>::length(start_str_arr[0]),
        std::char_traits<char>::length(start_str_arr[1]), std::char_traits<char>::length(start_str_arr[2]), std::char_traits<char>::length(start_str_arr[3]),
        std::char_traits<char>::length(start_str_arr[4]), std::char_traits<char>::length(start_str_arr[5]), std::char_traits<char>::length(start_str_arr[6]),
        std::char_traits<char>::length(start_str_arr[7]), std::char_traits<char>::length(start_str_arr[8]), std::char_traits<char>::length(start_str_arr[9]),
        std::char_traits<char>::length(start_str_arr[10]), std::char_traits<char>::length(start_str_arr[11]), std::char_traits<char>::length(start_str_arr[12]),
        std::char_traits<char>::length(start_str_arr[13]) };

    constexpr const char* seq_separator = "+";

    if(PrintScreenSaver::Get()->screenshot_key == key_code)
    {
        LOG(LogLevel::Warning, "Key \"{}\" is already assigned to PrintScreenSaver!");
        return;
    }
    if(PathSeparator::Get()->replace_key == key_code)
    {
        LOG(LogLevel::Warning, "Key \"{}\" is already assigned to PathSeparator!");
        return;
    }
    if(SymlinkCreator::Get()->mark_key == key_code || SymlinkCreator::Get()->place_symlink_key == key_code ||
        SymlinkCreator::Get()->place_hardlink_key == key_code)
    {
        LOG(LogLevel::Warning, "Key \"{}\" is already assigned to SymlinkCreator!");
        return;
    }
    if(bring_to_foreground_key == key_code)
    {
        LOG(LogLevel::Warning, "Key \"{}\" is already assigned to BringToForeground!");
        return;
    }

    size_t pos = 1;
    while(pos < str.length() - 1)
    {
        size_t first_end = str.find("]", pos + 1);
        size_t first_pos[MAX_ITEMS];
        for(std::underlying_type_t<MacroTypes> i = 0; i != MAX_ITEMS; ++i)
        {
            first_pos[i] = str.substr(0, first_end).find(start_str_arr[i], pos - 1);
        }

        uint8_t input_type = 0xFF;
        uint8_t not_empty_cnt = 0;
        for(std::underlying_type_t<MacroTypes> i = 0; i != MAX_ITEMS; ++i)
        {
            if(first_pos[i] != std::string::npos)
            {
                input_type = i;
                not_empty_cnt++;
            }
        }

        if(not_empty_cnt > 1 || input_type == 0xFF)
        {
            LOG(LogLevel::Error, "Error with config file macro formatting: {}", str);
            return;
        }

        switch(input_type)
        {
            case MacroTypes::BIND_NAME:
            {
                pos = first_end;
                c->bind_name[key_code] = utils::extract_string(str, first_pos[MacroTypes::BIND_NAME], first_end, start_str_arr_lens[MacroTypes::BIND_NAME]);
                break;
            }
            case MacroTypes::KEY_SEQ:
            {
                pos = first_end;
                std::string&& sequence = utils::extract_string(str, first_pos[MacroTypes::KEY_SEQ], first_end, start_str_arr_lens[MacroTypes::KEY_SEQ]);
                c->key_vec[key_code].push_back(std::make_unique<KeyCombination>(std::move(sequence)));
                break;
            }
            case MacroTypes::KEY_TYPE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::KEY_TYPE], first_end, start_str_arr_lens[MacroTypes::KEY_TYPE]);
                c->key_vec[key_code].push_back(std::make_unique<KeyText>(std::move(sequence)));
                break;
            }
            case MacroTypes::DELAY:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::DELAY], first_end, start_str_arr_lens[MacroTypes::DELAY]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<KeyDelay>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for DELAY: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_MOVE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::MOUSE_MOVE], first_end, start_str_arr_lens[MacroTypes::MOUSE_MOVE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseMovement>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for MOUSE_MOVE: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_INTERPOLATE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::MOUSE_INTERPOLATE], first_end, start_str_arr_lens[MacroTypes::MOUSE_INTERPOLATE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseInterpolate>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for MOUSE_INTERPOLATE: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_PRESS:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::MOUSE_PRESS], first_end, start_str_arr_lens[MacroTypes::MOUSE_PRESS]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MousePress>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for MOUSE_PRESS: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_RELEASE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::MOUSE_RELEASE], first_end, start_str_arr_lens[MacroTypes::MOUSE_RELEASE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseRelease>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for MOUSE_RELEASE: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_CLICK:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::MOUSE_CLICK], first_end, start_str_arr_lens[MacroTypes::MOUSE_CLICK]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseClick>(std::move(sequence)));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "Invalid argument for MOUSE_CLICK: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::BASH:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::BASH], first_end, start_str_arr_lens[MacroTypes::BASH]);
                c->key_vec[key_code].push_back(std::make_unique<BashCommand>(std::move(sequence)));
                break;
            }
            case MacroTypes::CMD:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::CMD], first_end, start_str_arr_lens[MacroTypes::CMD]);
                c->key_vec[key_code].push_back(std::make_unique<CommandExecute>(std::move(sequence)));
                break;
            }
            case MacroTypes::CMD_XML:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::CMD_XML], first_end, start_str_arr_lens[MacroTypes::CMD_XML]);
                c->key_vec[key_code].push_back(std::make_unique<CommandXml>(std::move(sequence)));
                break;
            }
            case MacroTypes::CMD_FG:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::CMD_FG], first_end, start_str_arr_lens[MacroTypes::CMD_FG]);
                c->key_vec[key_code].push_back(std::make_unique<KeyBringAppToForeground>(std::move(sequence)));
                break;
            }
            case MacroTypes::CMD_IMG:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MacroTypes::CMD_IMG], first_end, start_str_arr_lens[MacroTypes::CMD_IMG]);
                c->key_vec[key_code].push_back(std::make_unique<KeyFindImageOnScreen>(std::move(sequence)));
                break;
            }
            default:
            {
                LOG(LogLevel::Error, "Invalid sequence/text format in line: {}", str.c_str());
                break;
            }
        }
    }

    if(c->bind_name[key_code].empty())
    {
        c->bind_name[key_code] = "Unknown macro"; /* this needed because wxTreeList won't show empty string as row */
        LOG(LogLevel::Warning, "Macro name for key {} missing. Giving it 'Unknown macro', feel free to change it.", key_code);
    }
}

void CustomMacro::SimulateKeypress(const std::string& key)
{
    pressed_keys = key;
    ExecuteKeypresses();
    pressed_keys.clear();
}

void CustomMacro::ProcessReceivedData(const char* data, unsigned int len)
{
    if(len != sizeof(KeyData_t))
    {
        LOG(LogLevel::Verbose, "Data received with invalid length! ({}), expected: {}", len, sizeof(KeyData_t));
        std::string hex;
        utils::ConvertHexBufferToString(data, len, hex);
        LOG(LogLevel::Verbose, "Full Data buffer: {}", hex);
        return;
    }

    KeyData_t* k = (KeyData_t*)data;
    uint16_t crc = utils::crc16_modbus((void*)data, len - sizeof(KeyData_t::crc));
    if(!strncmp(data, "reset", 5) && crc == 0x6bd8) /* in case of suddenly reset of STM32 */
    {
        LOG(LogLevel::Verbose, "Reset received");
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
            ExecuteKeypresses();
        }
    }
    else
    {
        LOG(LogLevel::Verbose, "CRC mismatch, received {:X} != {:X}", k->crc, crc);
        std::string hex;
        utils::ConvertHexBufferToString(data, len, hex);
        LOG(LogLevel::Verbose, "Full Data buffer: {}", hex);
    }
}

uint16_t CustomMacro::GetKeyScanCode(const std::string& str)
{
    uint16_t ret = 0xFFFF;
    auto it = scan_codes.find(str);
    if(it != scan_codes.end())
        ret = it->second;
    return ret;
}

std::string CustomMacro::GetKeyStringFromScanCode(int scancode)
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

void CustomMacro::ExecuteKeypresses()
{
    std::scoped_lock lock(executor_mtx);
    if(PrintScreenSaver::Get()->screenshot_key == pressed_keys)
    {
        PrintScreenSaver::Get()->SaveScreenshot();
        return;
    }
    if(PathSeparator::Get()->replace_key == pressed_keys)
    {
        PathSeparator::Get()->ReplaceClipboard(PathSeparator::ReplaceType::PATH_SEPARATOR);
        return;
    }
    if(SymlinkCreator::Get()->HandleKeypress(pressed_keys))
        return;

    if(bring_to_foreground_key == pressed_keys)
    {
        ExecuteForegroundKeypress();
        return;
    }

    auto ExecuteGlobalMacro = [this]()
    {
        if(macros.empty())
            return;

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

void CustomMacro::ExecuteForegroundKeypress()
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(frame)
        frame->ToggleForegroundVisibility();
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