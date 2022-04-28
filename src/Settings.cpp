#include "pch.hpp"

void Settings::ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroAppProfile>& c)
{
    constexpr std::underlying_type_t<MacroTypes> MAX_ITEMS = MacroTypes::MAX;
    constexpr const char* start_str_arr[MAX_ITEMS] = { "BIND_NAME[", "KEY_SEQ[", "KEY_TYPE[", "DELAY[", "MOUSE_MOVE[", "MOUSE_INTERPOLATE[", 
        "MOUSE_PRESS[", "MOUSE_RELEASE", "MOUSE_CLICK[", "CMD[" };
    constexpr const size_t start_str_arr_lens[MAX_ITEMS] = { std::char_traits<char>::length(start_str_arr[0]),
        std::char_traits<char>::length(start_str_arr[1]), std::char_traits<char>::length(start_str_arr[2]), std::char_traits<char>::length(start_str_arr[3]), 
        std::char_traits<char>::length(start_str_arr[4]), std::char_traits<char>::length(start_str_arr[5]), std::char_traits<char>::length(start_str_arr[6]),
        std::char_traits<char>::length(start_str_arr[7]), std::char_traits<char>::length(start_str_arr[8]), std::char_traits<char>::length(start_str_arr[9]) };

    constexpr const char* seq_separator = "+";

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
            LOGMSG(error, "Error with config file macro formatting: {}", str);
            return;
        }

        switch(input_type)
        {
            case MacroTypes::BIND_NAME:
            {
                pos = first_end;
                c->bind_name[key_code] = utils::extract_string(str, first_pos[BIND_NAME], first_end, start_str_arr_lens[BIND_NAME]);
                break;
            }
            case MacroTypes::KEY_SEQ:
            {
                pos = first_end;
                std::string &&sequence = utils::extract_string(str, first_pos[KEY_SEQ], first_end, start_str_arr_lens[KEY_SEQ]);
                c->key_vec[key_code].push_back(std::make_unique<KeyCombination>(std::move(sequence)));
                break;
            }
            case MacroTypes::KEY_TYPE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[KEY_TYPE], first_end, start_str_arr_lens[KEY_TYPE]);
                c->key_vec[key_code].push_back(std::make_unique<KeyText>(std::move(sequence)));
                break;
            }
            case MacroTypes::DELAY:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[DELAY], first_end, start_str_arr_lens[DELAY]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<KeyDelay>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for DELAY: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_MOVE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MOUSE_MOVE], first_end, start_str_arr_lens[MOUSE_MOVE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseMovement>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for MOUSE_MOVE: {} ({})", sequence, e.what());
                }
                break;
            }            
            case MacroTypes::MOUSE_INTERPOLATE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MOUSE_INTERPOLATE], first_end, start_str_arr_lens[MOUSE_INTERPOLATE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseInterpolate>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for MOUSE_INTERPOLATE: {} ({})", sequence, e.what());
                }
                break;
            }
            case MacroTypes::MOUSE_PRESS:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MOUSE_PRESS], first_end, start_str_arr_lens[MOUSE_PRESS]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MousePress>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for MOUSE_PRESS: {} ({})", sequence, e.what());
                }
                break;
            }            
            case MacroTypes::MOUSE_RELEASE:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MOUSE_RELEASE], first_end, start_str_arr_lens[MOUSE_RELEASE]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseRelease>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for MOUSE_RELEASE: {} ({})", sequence, e.what());
                }
                break;
            }     
            case MacroTypes::MOUSE_CLICK:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[MOUSE_CLICK], first_end, start_str_arr_lens[MOUSE_CLICK]);
                try
                {
                    c->key_vec[key_code].push_back(std::make_unique<MouseClick>(std::move(sequence)));
                }
                catch(std::exception& e)
                {
                    LOGMSG(critical, "Invalid argument for MOUSE_CLICK: {} ({})", sequence, e.what());
                }
                break;
            }            
            case MacroTypes::COMMAND:
            {
                pos = first_end;
                std::string sequence = utils::extract_string(str, first_pos[COMMAND], first_end, start_str_arr_lens[COMMAND]);
                c->key_vec[key_code].push_back(std::make_unique<CommandExecute>(std::move(sequence)));
                break;
            }
            default:
            {
                LOGMSG(error, "Invalid sequence/text format in line: {}", str.c_str());
                break;
            }
        }
    }

    if(c->bind_name[key_code].empty())
    {
        c->bind_name[key_code] = "Unknown macro"; /* this needed because wxTreeList won't show empty string as row */
        LOGMSG(warning, "Macro name for key {} missing. Giving it 'Unknown macro', feel free to change it.", key_code);
    }
}

void Settings::LoadFile()
{
    if(!std::filesystem::exists("settings.ini"))
    {
        SaveFile(true);
    }

    boost::property_tree::ptree pt;
    try
    {
        boost::property_tree::ini_parser::read_ini("settings.ini", pt);
    }
    catch(boost::property_tree::ptree_error& e)
    {
        LOGMSG(error, "exception: {}", e.what());
    }

    try
    {
        CustomMacro::Get()->use_per_app_macro = utils::stob(pt.get_child("Macro_Config").find("UsePerApplicationMacros")->second.data());
        CustomMacro::Get()->advanced_key_binding = utils::stob(pt.get_child("Macro_Config").find("UseAdvancedKeyBinding")->second.data());

        CustomMacro::Get()->macros.clear();
        std::unique_ptr<MacroAppProfile> p = std::make_unique<MacroAppProfile>();
        macro_section.clear();
        auto& global_child = pt.get_child("Keys_Global");
        for(auto& key : global_child)
        {
            std::string& str = key.second.data();
            ParseMacroKeys(0, key.first, str, p);
        }
        p->app_name = "Global";
        CustomMacro::Get()->macros.push_back(std::move(p));

        /* load per-application macros */
        size_t counter = 1;
        size_t cnt = 0;
        while((cnt = pt.count("Keys_Macro" + std::to_string(counter))) == 1)
        {
            std::unique_ptr<MacroAppProfile> p2 = std::make_unique<MacroAppProfile>();
            auto& ch = pt.get_child("Keys_Macro" + std::to_string(counter));
            for(auto& key : ch)
            {
                if(key.first.data() == std::string("AppName"))
                {
                    p2->app_name = key.second.data();
                    continue;
                }
                std::string& str = key.second.data();
                ParseMacroKeys(counter, key.first, str, p2);
            }
            counter++;
            CustomMacro::Get()->macros.push_back(std::move(p2));
        }
        SerialPort::Get()->SetEnabled(utils::stob(pt.get_child("COM_Backend").find("Enable")->second.data()));
        SerialPort::Get()->SetComPort(utils::stoi<uint16_t>(pt.get_child("COM_Backend").find("COM")->second.data()));
        SerialPort::Get()->SetForwardToTcp(utils::stob(pt.get_child("COM_Backend").find("ForwardViaTcp")->second.data()));
        SerialPort::Get()->SetRemoteTcpIp(pt.get_child("COM_Backend").find("RemoteTcpIp")->second.data());
        SerialPort::Get()->SetRemoteTcpPort(utils::stoi<uint16_t>(pt.get_child("COM_Backend").find("RemoteTcpPort")->second.data()));

        SerialForwarder::Get()->is_enabled = utils::stob(pt.get_child("COM_TcpBackend").find("Enable")->second.data());
        SerialForwarder::Get()->bind_ip = std::move(pt.get_child("COM_TcpBackend").find("ListeningIp")->second.data());
        SerialForwarder::Get()->tcp_port = utils::stoi<uint16_t>(pt.get_child("COM_TcpBackend").find("ListeningPort")->second.data());

        Server::Get()->is_enabled = utils::stob(pt.get_child("TCP_Backend").find("Enable")->second.data());
        Server::Get()->tcp_port = utils::stoi<uint16_t>(pt.get_child("TCP_Backend").find("TCP_Port")->second.data());
        minimize_on_exit = utils::stob(pt.get_child("App").find("MinimizeOnExit")->second.data());
        minimize_on_startup = utils::stob(pt.get_child("App").find("MinimizeOnStartup")->second.data());
        default_page = utils::stoi<decltype(default_page)>(pt.get_child("App").find("DefaultPage")->second.data());
        remember_window_size = utils::stoi<decltype(remember_window_size)>(pt.get_child("App").find("RememberWindowSize")->second.data());
        if(remember_window_size)
        {
            if(sscanf(pt.get_child("App").find("LastWindowSize")->second.data().c_str(), "%d,%d", &window_size.x, &window_size.y) != 2)
                LOGMSG(error, "Invalid ini format for WindowSize");

            if(window_size.x < WINDOW_SIZE_X) 
                window_size.x = WINDOW_SIZE_X;
            if(window_size.y < WINDOW_SIZE_Y)
                window_size.y = WINDOW_SIZE_Y;
        }
        always_on_numlock = utils::stob(pt.get_child("App").find("AlwaysOnNumLock")->second.data());

        PrintScreenSaver::Get()->screenshot_key = std::move(pt.get_child("Screenshot").find("ScreenshotKey")->second.data());
        PrintScreenSaver::Get()->timestamp_format = std::move(pt.get_child("Screenshot").find("ScreenshotDateFormat")->second.data());
        PrintScreenSaver::Get()->screenshot_path = std::move(pt.get_child("Screenshot").find("ScreenshotPath")->second.data());
        PathSeparator::Get()->replace_key = std::move(pt.get_child("PathSeparator").find("ReplacePathSeparatorKey")->second.data());

        std::error_code ec;
        if(!std::filesystem::exists(PrintScreenSaver::Get()->screenshot_path))
            std::filesystem::create_directory(PrintScreenSaver::Get()->screenshot_path, ec);
        if(ec)
            LOGMSG(error, "Error with create_directory ({}): {}", PrintScreenSaver::Get()->screenshot_path.generic_string(), ec.message());

        SymlinkCreator::Get()->is_enabled = utils::stob(pt.get_child("SymlinkCreator").find("Enable")->second.data());
        SymlinkCreator::Get()->mark_key = std::move(pt.get_child("SymlinkCreator").find("MarkKey")->second.data());
        SymlinkCreator::Get()->place_symlink_key = std::move(pt.get_child("SymlinkCreator").find("PlaceSymlinkKey")->second.data());
        SymlinkCreator::Get()->place_hardlink_key = std::move(pt.get_child("SymlinkCreator").find("PlaceHardlinkKey")->second.data());

        AntiLock::Get()->is_enabled = utils::stob(pt.get_child("AntiLock").find("Enable")->second.data());
        AntiLock::Get()->timeout = utils::stoi<uint32_t>(pt.get_child("AntiLock").find("Timeout")->second.data());
        AntiLock::Get()->is_screensaver = utils::stob(pt.get_child("AntiLock").find("StartScreenSaver")->second.data());
        std::vector<std::string> ignore_list;
        boost::split(ignore_list, pt.get_child("AntiLock").find("Exclusions")->second.data(), boost::is_any_of("|"));
        AntiLock::Get()->exclusions = std::move(ignore_list);

        TerminalHotkey::Get()->is_enabled = utils::stob(pt.get_child("TerminalHotkey").find("Enable")->second.data());
        const std::string& key = pt.get_child("TerminalHotkey").find("Key")->second.data();
        TerminalHotkey::Get()->vkey = utils::GetVirtualKeyFromString(key);
        if(TerminalHotkey::Get()->vkey == 0xFFFF)
        {
            LOGMSG(warning, "Invalid hotkey was specified for TerminalHotkey: {}", key);
            TerminalHotkey::Get()->is_enabled = false;
        }

        DirectoryBackup::Get()->backup_time_format = std::move(pt.get_child("BackupSettings").find("BackupFileFormat")->second.data());

        /* load backup configs */
        DirectoryBackup::Get()->Clear();
        size_t counter_ = 1;
        size_t cnt_ = 0;
        while((cnt_ = pt.count("Backup_" + std::to_string(counter_))) == 1)
        {
            std::string key = "Backup_" + std::to_string(counter_);

            std::filesystem::path from = pt.get_child(key).find("From")->second.data();

            std::vector<std::filesystem::path> to;
            boost::split(to, pt.get_child(key).find("To")->second.data(), boost::is_any_of("|"));

            std::vector<std::wstring> ignore_list;
            std::wstring ignore;
            utils::MBStringToWString(pt.get_child(key).find("Ignore")->second.data(), ignore);
            boost::split(ignore_list, ignore, boost::is_any_of("|"));
            int max_backups = utils::stoi<decltype(max_backups)>(pt.get_child(key).find("MaxBackups")->second.data());
            bool calculate_hash = utils::stob(pt.get_child(key).find("CalculateHash")->second.data());
            size_t buffer_size = utils::stob(pt.get_child(key).find("BufferSize")->second.data());
            BackupEntry* b = new BackupEntry(std::move(from), std::move(to), std::move(ignore_list), max_backups, calculate_hash, buffer_size);

            counter_++;
            DirectoryBackup::Get()->backups.push_back(b);
        }

        uint32_t val1 = utils::stoi<decltype(val1)>(pt.get_child("Graph").find("Graph1HoursBack")->second.data());
        DatabaseLogic::Get()->SetGraphHours(0, val1);
        uint32_t val2 = utils::stoi<decltype(val1)>(pt.get_child("Graph").find("Graph2HoursBack")->second.data());
        DatabaseLogic::Get()->SetGraphHours(1, val2);
    }
    catch(boost::property_tree::ptree_error& e)
    {
        LOGMSG(error, "Ptree exception: {}", e.what());
    }
    catch(std::exception& e)
    {
        LOGMSG(critical, "Exception {}", e.what());
    }
}

void Settings::SaveFile(bool write_default_macros) /* tried boost::ptree ini writer but it doesn't support comments... sticking to plain file functions */
{
    std::ofstream out("settings.ini", std::ofstream::binary);
    out << "# Possible macro keywords: \n";
    out << "# BIND_NAME[binding name] = Set the name if macro. Should be used as first\n";
    out << "# KEY_TYPE[text] = Press & release given keys in sequence to type a text\n";
    out << "# KEY_SEQ[CTRL+C] = Press all given keys after each other and release it when each was pressed - ideal for key shortcats\n";
    out << "# DELAY[time in ms] = Waits for given milliseconds\n";
    out << "# DELAY[min ms - max ms] = Waits randomly between min ms and max ms\n";
    out << "# MOUSE_MOVE[x,y] = Move mouse to given coordinates\n";
    out << "# MOUSE_INTERPOLATE[x,y] = Move mouse with interpolation to given coordinates\n";
    out << "# MOUSE_PRESS[key] = Press given mouse key\n";
    out << "# MOUSE_RELEASE[key] = Release given mouse key\n";
    out << "# MOUSE_CLICK[key] = Click (press and release) with mouse\n";
    out << "# CMD[key] = Execute specified command with command line\n";
    out << "\n";
    out << "[Macro_Config]\n";
    out << "# Use per-application macros. AppName is searched in active window title, so window name must contain AppName\n";
    out << "UsePerApplicationMacros = " << CustomMacro::Get()->use_per_app_macro << "\n";
    out << "\n";
    out << "# If enabled, you can bind multiple key combinations with special keys like RSHIFT + 1, but can't bind SHIFT, CTRL and other special keys alone\n";
    out << "UseAdvancedKeyBinding = " << CustomMacro::Get()->advanced_key_binding << "\n";
    out << "\n";

    if(!write_default_macros)  /* True if settings.ini file doesn't exists - write a few macro lines here as example */
    {
        int cnt = 0;
        std::string key;
        auto& m = CustomMacro::Get()->GetMacros();
        for(auto& i : m)
        {
            if(!cnt)
                out << "[Keys_Global]\n";
            else
            {
                out << fmt::format("\n[Keys_Macro{}]\n", cnt);
                out << fmt::format("AppName = {}\n", i->app_name);
            }
            cnt++;
            for(auto& x : i->key_vec)
            {
                key = fmt::format("{} = BIND_NAME[{}]", x.first, i->bind_name[x.first]);

                for(auto& k : x.second)
                {
                    IKey* p = k.get();
                    key += p->GenerateText(true);
                }
                out << key << '\n';
                key.clear();
            }
        }
    }
    else
    {
        out << "[Keys_Global]\n";
        out << "NUM_0 = BIND_NAME[global macro 1] KEY_SEQ[A+B+C]\n";
        out << "NUM_1 = BIND_NAME[global macro 2] KEY_TYPE[global macro 1]\n";
        out << "\n";
        out << "[Keys_Macro1]\n";
        out << "AppName = Notepad\n";
        out << "NUM_1 = BIND_NAME[close notepad++] KEY_TYPE[test string from CustomKeyboard.exe] DELAY[100] KEY_TYPE[Closing window...] DELAY[100-3000] KEY_SEQ[LALT+F4] DELAY[100] KEY_SEQ[RIGHT] KEY_SEQ[ENTER]\n";
    }

    out << "\n";
    out << "[TCP_Backend]\n";
    out << "Enable = " << Server::Get()->is_enabled << "\n";
    out << "TCP_Port = " << Server::Get()->tcp_port << " # TCP Port for receiving measurements from sensors\n";
    out << "\n";
    out << "[COM_Backend]\n";
    out << "Enable = " << SerialPort::Get()->IsEnabled() << "\n";
    out << "COM = " << SerialPort::Get()->GetComPort() << " # Com port for UART where the data is received from STM32\n";
    out << "ForwardViaTcp = " << SerialPort::Get()->IsForwardToTcp() << " # Is data have to be forwarded to remote TCP server\n";
    out << "RemoteTcpIp = " << SerialPort::Get()->GetRemoteTcpIp() << "\n";
    out << "RemoteTcpPort = " << SerialPort::Get()->GetRemoteTcpPort() << "\n";
    out << "[COM_TcpBackend]\n";
    out << "Enable = " << SerialForwarder::Get()->is_enabled << " # Listening port from second instance where the TCP Forwarder forwards data received from COM port\n";
    out << "ListeningIp = " << SerialForwarder::Get()->bind_ip << "\n";
    out << "ListeningPort = " << SerialForwarder::Get()->tcp_port << "\n";
    out << "\n";
    out << "[App]\n";
    out << "MinimizeOnExit = " << minimize_on_exit << "\n";
    out << "MinimizeOnStartup = " << minimize_on_startup<< "\n";
    out << "DefaultPage = " << static_cast<uint16_t>(default_page) << "\n";
    out << "RememberWindowSize = " << remember_window_size << "\n";
    if(remember_window_size)  /* get frame size when click on Save - not on exit, this is not a bug */
    {
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        window_size = frame->GetSize();
    }
    out << "LastWindowSize = " << fmt::format("{}, {}", window_size.x, window_size.y) << "\n";
    out << "AlwaysOnNumLock = " << always_on_numlock << "\n";
    out << "\n";
    out << "[Screenshot]\n";
    out << "ScreenshotKey = " << PrintScreenSaver::Get()->screenshot_key << "\n";
    out << "ScreenshotDateFormat = " << PrintScreenSaver::Get()->timestamp_format << "\n";
    out << "ScreenshotPath = " << PrintScreenSaver::Get()->screenshot_path.generic_string() << "\n";
    out << "\n";
    out << "[PathSeparator]\n";
    out << "ReplacePathSeparatorKey = " << PathSeparator::Get()->replace_key << "\n";
    out << "\n";
    out << "[SymlinkCreator]\n";
    out << "Enable = " << SymlinkCreator::Get()->is_enabled << "\n";
    out << "MarkKey = " << SymlinkCreator::Get()->mark_key << "\n";
    out << "PlaceSymlinkKey = " << SymlinkCreator::Get()->place_symlink_key << "\n";
    out << "PlaceHardlinkKey = " << SymlinkCreator::Get()->place_hardlink_key << "\n";
    out << "\n";
    out << "[AntiLock]\n";
    out << "Enable = " << AntiLock::Get()->is_enabled << "\n";
    out << "Timeout = " << AntiLock::Get()->timeout << " # Seconds\n";
    out << "StartScreenSaver = " << AntiLock::Get()->is_screensaver << "\n";
    std::string exclusions;
    for(auto& x : AntiLock::Get()->exclusions)
    {
        exclusions += x + '|';
    }
    if(exclusions.back() == '|')
        exclusions.pop_back();
    out << "Exclusions = " << exclusions << '\n';
    out << "\n";
    out << "[TerminalHotkey]\n";
    out << "Enable = " << TerminalHotkey::Get()->is_enabled << "\n";
    out << "Key = " << utils::GetKeyStringFromVirtualKey(TerminalHotkey::Get()->vkey) << "\n";
    out << "[BackupSettings]\n";
    out << "BackupFileFormat = " << DirectoryBackup::Get()->backup_time_format << "\n";
    if(!write_default_macros)
    {
        int cnt = 1;
        std::wstring key;
        for(auto& i : DirectoryBackup::Get()->backups)
        {
            out << fmt::format("\n[Backup_{}]\n", cnt++);
            out << "From = " << i->from.generic_wstring() << '\n';
            key.clear();
            for(auto& x : i->to)
            {
                key += x.generic_wstring() + '|';
            }
            if(key.back() == '|')
                key.pop_back();
            out << "To = " << key << '\n';
            key.clear();
            for(auto& x : i->ignore_list)
            {
                key += x + '|';
            }
            if(key.back() == '|')
                key.pop_back();

            std::string ignore_list;
            utils::WStringToMBString(key, ignore_list);
            out << "Ignore = " << ignore_list << '\n';
            out << "MaxBackups = " << i->max_backups << '\n';
            out << "CalculateHash = " << i->calculate_hash << '\n';
            out << "BufferSize = " << i->hash_buf_size << " # Buffer size for file operations - determines how much data is read once, Unit: Megabytes" << '\n';
        }
    }
    else
    {
        out << "\n[Backup_1]\n";
        out << "From = C:\\Users\\Ati\\Desktop\\folder_from_backup\n";
        out << "To = C:\\Users\\Ati\\Desktop\\folder_where_to_backup|F:\\Backup\\folder_where_to_backup\n";
        out << "Ignore = git/COMMIT_EDITMSG|.git|.vs|Debug|Release|Screenshots|x64|Graphs/Line Chart|Graphs/Temperature.html|Graphs/Humidity.html|Graphs/CO2.html|Graphs/Lux.html|Graphs/VOC.html|Graphs/CCT.html|Graphs/PM10.html|Graphs/PM25.html\n";
        out << "MaxBackups = 5\n";
        out << "CalculateHash = 1\n";
        out << "BufferSize = 2\n";
    }
    out << "\n";
    out << "[Graph]\n";
    out << "Graph1HoursBack = " << DatabaseLogic::Get()->GetGraphHours(0) << " # One day\n";
    out << "Graph2HoursBack = " << DatabaseLogic::Get()->GetGraphHours(1) << " # One week\n";
}

void Settings::Init()
{
    LoadFile();
}