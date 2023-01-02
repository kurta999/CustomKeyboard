#include "pch.hpp"

static constexpr const char* SETTINGS_FILE_PATH = "./settings.ini";

void Settings::LoadFile()
{
    if(!std::filesystem::exists(SETTINGS_FILE_PATH))
    {
        SaveFile(true);
        LOG(LogLevel::Normal, "Default {} is missing, creating one", SETTINGS_FILE_PATH);
    }

    boost::property_tree::ptree pt;
    try
    {
        boost::property_tree::ini_parser::read_ini(SETTINGS_FILE_PATH, pt);
    }
    catch(boost::property_tree::ptree_error& e)
    {
        LOG(LogLevel::Critical, "Exception: {}", e.what());
    }

    try
    {
        CustomMacro::Get()->use_per_app_macro = utils::stob(pt.get_child("Macro_Config").find("UsePerApplicationMacros")->second.data());
        CustomMacro::Get()->advanced_key_binding = utils::stob(pt.get_child("Macro_Config").find("UseAdvancedKeyBinding")->second.data());
        CustomMacro::Get()->bring_to_foreground_key = std::move(pt.get_child("Macro_Config").find("BringToForegroundKey")->second.data());

        CustomMacro::Get()->macros.clear();
        std::unique_ptr<MacroAppProfile> p = std::make_unique<MacroAppProfile>();
        auto& global_child = pt.get_child("Keys_Global");
        for(auto& key : global_child)
        {
            std::string& str = key.second.data();
            CustomMacro::Get()->ParseMacroKeys(0, key.first, str, p);
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
                CustomMacro::Get()->ParseMacroKeys(counter, key.first, str, p2);
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

        CanEntryHandler* can_handler = wxGetApp().can_entry;
        CanSerialPort::Get()->SetEnabled(utils::stob(pt.get_child("CANSender").find("Enable")->second.data()));
        CanSerialPort::Get()->SetComPort(utils::stoi<uint16_t>(pt.get_child("CANSender").find("COM")->second.data()));
        CanSerialPort::Get()->SetDeviceType(static_cast<CanDeviceType>(utils::stoi<uint8_t>(pt.get_child("CANSender").find("DeviceType")->second.data())));
        can_handler->ToggleAutoSend(utils::stob(pt.get_child("CANSender").find("AutoSend")->second.data()));
        can_handler->ToggleAutoRecord(utils::stob(pt.get_child("CANSender").find("AutoRecord")->second.data()));
        can_handler->SetRecordingLogLevel(utils::stoi<uint8_t>(pt.get_child("CANSender").find("DefaultRecordingLogLevel")->second.data()));
        can_handler->default_tx_list = std::move(pt.get_child("CANSender").find("DefaultTxList")->second.data());
        can_handler->default_rx_list = pt.get_child("CANSender").find("DefaultRxList")->second.data();
        can_handler->default_mapping = pt.get_child("CANSender").find("DefaultMapping")->second.data();
        ModbusMasterSerialPort::Get()->SetEnabled(utils::stob(pt.get_child("ModbusMaster").find("Enable")->second.data()));
        ModbusMasterSerialPort::Get()->SetComPort(utils::stoi<uint16_t>(pt.get_child("ModbusMaster").find("COM")->second.data()));

        minimize_on_exit = utils::stob(pt.get_child("App").find("MinimizeOnExit")->second.data());
        minimize_on_startup = utils::stob(pt.get_child("App").find("MinimizeOnStartup")->second.data());
        default_page = utils::stoi<decltype(default_page)>(pt.get_child("App").find("DefaultPage")->second.data());
        remember_window_size = utils::stoi<decltype(remember_window_size)>(pt.get_child("App").find("RememberWindowSize")->second.data());
        if(remember_window_size)
        {
            if(sscanf(pt.get_child("App").find("LastWindowSize")->second.data().c_str(), "%d,%d", &window_size.x, &window_size.y) != 2)
                LOG(LogLevel::Error, "Invalid ini format for WindowSize");

            if(window_size.x < WINDOW_SIZE_X)
                window_size.x = WINDOW_SIZE_X;
            if(window_size.y < WINDOW_SIZE_X)
                window_size.y = WINDOW_SIZE_Y;
        }
        always_on_numlock = utils::stob(pt.get_child("App").find("AlwaysOnNumLock")->second.data());
        shared_drive_letter = pt.get_child("App").find("SharedDriveLetter")->second.data()[0];

        PrintScreenSaver::Get()->screenshot_key = std::move(pt.get_child("Screenshot").find("ScreenshotKey")->second.data());
        PrintScreenSaver::Get()->timestamp_format = std::move(pt.get_child("Screenshot").find("ScreenshotDateFormat")->second.data());
        PrintScreenSaver::Get()->screenshot_path = std::move(pt.get_child("Screenshot").find("ScreenshotPath")->second.data());
        PathSeparator::Get()->replace_key = std::move(pt.get_child("PathSeparator").find("ReplacePathSeparatorKey")->second.data());

        std::error_code ec;
        if(!std::filesystem::exists(PrintScreenSaver::Get()->screenshot_path))
            std::filesystem::create_directory(PrintScreenSaver::Get()->screenshot_path, ec);
        if(ec)
            LOG(LogLevel::Error, "Error with create_directory ({}): {}", PrintScreenSaver::Get()->screenshot_path.generic_string(), ec.message());

        SymlinkCreator::Get()->is_enabled = utils::stob(pt.get_child("SymlinkCreator").find("Enable")->second.data());
        SymlinkCreator::Get()->mark_key = std::move(pt.get_child("SymlinkCreator").find("MarkKey")->second.data());
        SymlinkCreator::Get()->place_symlink_key = std::move(pt.get_child("SymlinkCreator").find("PlaceSymlinkKey")->second.data());
        SymlinkCreator::Get()->place_hardlink_key = std::move(pt.get_child("SymlinkCreator").find("PlaceHardlinkKey")->second.data());

        AntiLock::Get()->is_enabled = utils::stob(pt.get_child("AntiLock").find("Enable")->second.data());
        AntiLock::Get()->timeout = utils::stoi<uint32_t>(pt.get_child("AntiLock").find("Timeout")->second.data());
        AntiLock::Get()->is_screensaver = utils::stob(pt.get_child("AntiLock").find("StartScreenSaver")->second.data());
        AntiLock::Get()->LoadExclusions(pt.get_child("AntiLock").find("Exclusions")->second.data());

        TerminalHotkey::Get()->is_enabled = utils::stob(pt.get_child("TerminalHotkey").find("Enable")->second.data());
        const std::string& key = pt.get_child("TerminalHotkey").find("Key")->second.data();
        TerminalHotkey::Get()->SetKey(key);
        TerminalHotkey::Get()->type = static_cast<TerminalType>(utils::stoi<uint8_t>(pt.get_child("TerminalHotkey").find("Type")->second.data()));

        IdlePowerSaver::Get()->is_enabled = utils::stob(pt.get_child("IdlePowerSaver").find("Enable")->second.data());
        IdlePowerSaver::Get()->timeout = utils::stoi<uint32_t>(pt.get_child("IdlePowerSaver").find("Timeout")->second.data());
        IdlePowerSaver::Get()->reduced_power_percent = utils::stoi<uint8_t>(pt.get_child("IdlePowerSaver").find("ReducedPowerPercent")->second.data());

        DirectoryBackup::Get()->backup_time_format = std::move(pt.get_child("BackupSettings").find("BackupFileFormat")->second.data());

        /* load backup configs */
        DirectoryBackup::Get()->Clear();
        size_t backup_counter = 1;
        size_t cnt_ = 0;
        while((cnt_ = pt.count("Backup_" + std::to_string(backup_counter))) == 1)
        {
            std::string key = "Backup_" + std::to_string(backup_counter);

            DirectoryBackup::Get()->LoadEntry(pt.get_child(key).find("From")->second.data(), pt.get_child(key).find("To")->second.data(),
                pt.get_child(key).find("Ignore")->second.data(), utils::stoi<size_t>(pt.get_child(key).find("MaxBackups")->second.data()),
                utils::stob(pt.get_child(key).find("CalculateHash")->second.data()), utils::stob(pt.get_child(key).find("BufferSize")->second.data()));
            backup_counter++;
        }

        uint32_t val1 = utils::stoi<decltype(val1)>(pt.get_child("Graph").find("Graph1HoursBack")->second.data());
        DatabaseLogic::Get()->SetGraphHours(0, val1);
        uint32_t val2 = utils::stoi<decltype(val1)>(pt.get_child("Graph").find("Graph2HoursBack")->second.data());
        DatabaseLogic::Get()->SetGraphHours(1, val2);

        std::string pages_str = pt.get_child("App").find("UsedPages")->second.data();
        if(boost::icontains(pages_str, "Main"))
            used_pages.main = 1;
        if(boost::icontains(pages_str, "Config"))
            used_pages.config = 1;
        if(boost::icontains(pages_str, "wxEditor"))
            used_pages.wxeditor = 1;
        if(boost::icontains(pages_str, "Map"))
            used_pages.map_converter = 1;
        if(boost::icontains(pages_str, "StringEscaper"))
            used_pages.escaper = 1;
        if(boost::icontains(pages_str, "Debug"))
            used_pages.debug = 1;
        if(boost::icontains(pages_str, "StructParser"))
            used_pages.struct_parser = 1;
        if(boost::icontains(pages_str, "FileBrowser"))
            used_pages.file_browser = 1;
        if(boost::icontains(pages_str, "CmdExecutor"))
            used_pages.cmd_executor = 1;
        if(boost::icontains(pages_str, "CanSender"))
            used_pages.can = 1;
        if(boost::icontains(pages_str, "ModbusMaster"))
            used_pages.modbus_master = 1;
        if(boost::icontains(pages_str, "Log"))
            used_pages.log = 1;
    }
    catch(boost::property_tree::ptree_error& e)
    {
        LOG(LogLevel::Critical, "Ptree exception: {}", e.what());
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Critical, "Exception {}", e.what());
    }

    if(used_pages.pages == 0)  /* Enable at least the Log panel if everything is disabled */
    {
        used_pages.log = 1;
        LOG(LogLevel::Error, "Enabling log panel, because every panel is disabled");
    }
}

void Settings::SaveFile(bool write_default_macros) /* tried boost::ptree ini writer but it doesn't support comments... sticking to plain file functions */
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::ofstream out(SETTINGS_FILE_PATH, std::ofstream::binary);
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
    out << "# BASH[key] = Execute specified command(s) with command line and keeps terminal shown\n";
    out << "# CMD[key] = Execute specified command(s) with command line without terminal\n";
    out << "\n";
    out << "[Macro_Config]\n";
    out << "# Use per-application macros. AppName is searched in active window title, so window name must contain AppName\n";
    out << "UsePerApplicationMacros = " << CustomMacro::Get()->use_per_app_macro << "\n";
    out << "\n";
    out << "# If enabled, you can bind multiple key combinations with special keys like RSHIFT + 1, but can't bind SHIFT, CTRL and other special keys alone\n";
    out << "UseAdvancedKeyBinding = " << CustomMacro::Get()->advanced_key_binding << "\n";
    out << "\n";
    out << "# If set to valid key, pressing this key will bring this application to foreground or minimize it to the tray\n";
    out << "BringToForegroundKey = " << CustomMacro::Get()->bring_to_foreground_key << "\n";
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
                out << std::format("\n[Keys_Macro{}]\n", cnt);
                out << std::format("AppName = {}\n", i->app_name);
            }
            cnt++;
            for(auto& x : i->key_vec)
            {
                key = std::format("{} = BIND_NAME[{}]", x.first, i->bind_name[x.first]);

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
    out << "\n";
    out << "[COM_TcpBackend]\n";
    out << "Enable = " << SerialForwarder::Get()->is_enabled << " # Listening port from second instance where the TCP Forwarder forwards data received from COM port\n";
    out << "ListeningIp = " << SerialForwarder::Get()->bind_ip << "\n";
    out << "ListeningPort = " << SerialForwarder::Get()->tcp_port << "\n";
    out << "\n";
    out << "[CANSender]\n";
    out << "Enable = " << CanSerialPort::Get()->IsEnabled() << "\n";
    out << "COM = " << CanSerialPort::Get()->GetComPort() << " # Com port for CAN UART where data is received/sent from/to STM32\n";
    out << "DeviceType = " << static_cast<int>(CanSerialPort::Get()->GetDeviceType()) << " # 0 = STM32, 1 = LAWICEL\n";
    out << "AutoSend = " << can_handler->IsAutoSend() << "\n";
    out << "AutoRecord = " << can_handler->IsAutoRecord() << "\n";
    out << "DefaultRecordingLogLevel = " << static_cast<int>(can_handler->GetRecordingLogLevel()) << "\n";
    out << "DefaultTxList = " << can_handler->default_tx_list.generic_string() << "\n";
    out << "DefaultRxList = " << can_handler->default_rx_list.generic_string() << "\n";
    out << "DefaultMapping = " << can_handler->default_mapping.generic_string() << "\n";
    out << "\n";
    out << "[ModbusMaster]\n";
    out << "Enable = " << ModbusMasterSerialPort::Get()->IsEnabled() << "\n";
    out << "COM = " << ModbusMasterSerialPort::Get()->GetComPort() << " # Com port for Modbus Master UART where data is received/sent from/to Modbus\n";
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
    out << "LastWindowSize = " << std::format("{}, {}", window_size.x, window_size.y) << "\n";
    out << "AlwaysOnNumLock = " << always_on_numlock << "\n";
    out << "SharedDriveLetter = " << shared_drive_letter << "\n";
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
    out << "Exclusions = " << AntiLock::Get()->SaveExclusions() << '\n';
    out << "\n";
    out << "[TerminalHotkey]\n";
    out << "Enable = " << TerminalHotkey::Get()->is_enabled << "\n";
    out << "Key = " << TerminalHotkey::Get()->GetKey() << "\n";
    out << "Type = " << static_cast<uint32_t>(TerminalHotkey::Get()->type) << " # 0 = WINDOWS_TERMINAL, 1 = cmd.exe, 2 = POWER_SHELL, 3 = BASH_TERMINAL" << "\n";
    out << "\n";
    out << "[IdlePowerSaver]\n";
    out << "Enable = " << IdlePowerSaver::Get()->is_enabled << "\n";
    out << "Timeout = " << IdlePowerSaver::Get()->timeout << "\n";
    out << "ReducedPowerPercent = " << static_cast<int>(IdlePowerSaver::Get()->reduced_power_percent) << "\n";
    out << "\n";
    out << "[BackupSettings]\n";
    out << "BackupFileFormat = " << DirectoryBackup::Get()->backup_time_format << "\n";
    if(!write_default_macros)
    {
        int cnt = 1;
        std::wstring key;
        for(auto& i : DirectoryBackup::Get()->backups)
        {
            out << std::format("\n[Backup_{}]\n", cnt++);
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