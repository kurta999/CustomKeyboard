#include <boost/asio.hpp>

#include "Settings.h"
#include "Server.h"
#include "Logger.h"
#include "PrintScreenSaver.h"
#include "PathSeparator.h"
#include "DirectoryBackup.h"

#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <cstdint>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

#include <array>

inline std::string extract_string(std::string& str, size_t start, size_t start_end, size_t len)
{
    return str.substr(start + len, start_end - start - len);
}

void Settings::ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroContainer>& c)
{
    enum ConfigTypes : uint8_t
    {
        KEY_SEQ, KEY_TYPE, DELAY, MOUSE_MOVE, MOUSE_CLICK
    };

    constexpr size_t MAX_ITEMS = 5;
    constexpr const char* start_str_arr[MAX_ITEMS] = { "KEY_SEQ[", "KEY_TYPE[", "DELAY[", "MOUSE_MOVE[", "MOUSE_CLICK[" };
    constexpr const char start_str_arr_lens[MAX_ITEMS] = { std::char_traits<char>::length(start_str_arr[0]), std::char_traits<char>::length(start_str_arr[1]), 
        std::char_traits<char>::length(start_str_arr[2]), std::char_traits<char>::length(start_str_arr[3]), std::char_traits<char>::length(start_str_arr[4]) };

    constexpr const char* seq_separator = "+";

    size_t pos = 1;
    while(pos < str.length() - 1)
    {
        size_t first_end = str.find("]", pos + 1);
        size_t first_pos[5];
        for(int i = 0; i != MAX_ITEMS; ++i)
        {
            first_pos[i] = str.substr(0, first_end).find(start_str_arr[i], pos - 1);
        }

        uint8_t input_type = 0xFF;
        uint8_t not_empty_cnt = 0;
        for(int i = 0; i != MAX_ITEMS; ++i)
        {
            if(first_pos[i] != std::string::npos)
            {
                input_type = i;
                not_empty_cnt++;
            }
        }

        if(not_empty_cnt > 1 || input_type == 0xFF)
        {
            LOGMSG(error, "Error with config file macro formatting");
            return;
        }

        switch(input_type)
        {
            case ConfigTypes::KEY_SEQ:
            {
                pos = first_end;
                std::vector<uint16_t> keys;
                std::string sequence = extract_string(str, first_pos[KEY_SEQ], first_end, start_str_arr_lens[KEY_SEQ]);

                boost::char_separator<char> sep(seq_separator);
                boost::tokenizer< boost::char_separator<char> > tok(sequence, sep);
                for(boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
                {
                    DBG("Token: %s\n", beg->c_str());
                    std::string key_code = *beg;
                    uint16_t key = CustomMacro::Get()->GetKeyScanCode(key_code);
                    keys.push_back(key);
                }

                c->key_vec[key_code].push_back(std::make_unique<KeyCombination>(std::move(keys)));
                break;
            }
            case ConfigTypes::KEY_TYPE:
            {
                pos = first_end;
                std::string sequence = extract_string(str, first_pos[KEY_TYPE], first_end, start_str_arr_lens[KEY_TYPE]);

                DBG("Text Token: %s\n", sequence.c_str());
                c->key_vec[key_code].push_back(std::make_unique<KeyText>(std::move(sequence)));
                break;
            }
            case ConfigTypes::DELAY:
            {
                pos = first_end;
                std::string sequence = extract_string(str, first_pos[DELAY], first_end, start_str_arr_lens[DELAY]);

                size_t separator_pos = sequence.find("-");
                if(separator_pos != std::string::npos)
                {
                    uint32_t delay_start = static_cast<uint32_t>(std::stoi(sequence));
                    uint32_t delay_end = static_cast<uint32_t>(std::stoi(&sequence[separator_pos + 1]));
                    DBG("Random Delay range: %d, %d\n", delay_start, delay_end);

                    c->key_vec[key_code].push_back(std::make_unique<KeyDelay>(delay_start, delay_end));
                }
                else
                {
                    uint32_t delay = static_cast<uint32_t>(std::stoi(sequence));
                    DBG("Delay: %d\n", delay);
                    c->key_vec[key_code].push_back(std::make_unique<KeyDelay>(delay));
                }
                break;
            }
            case ConfigTypes::MOUSE_MOVE:
            {
                pos = first_end;
                std::string sequence = extract_string(str, first_pos[MOUSE_MOVE], first_end, start_str_arr_lens[MOUSE_MOVE]);

                size_t separator_pos = sequence.find(",");
                if(separator_pos != std::string::npos)
                {
                    POINT pos;
                    pos.x = static_cast<long>(std::stoi(sequence));
                    pos.y = static_cast<uint32_t>(std::stoi(&sequence[separator_pos + 1]));
                    DBG("Mouse Movement: %d, %d\n", pos.x, pos.y);

                    c->key_vec[key_code].push_back(std::make_unique<MouseMovement>((LPPOINT*)&pos));
                }
                break;
            }
            case ConfigTypes::MOUSE_CLICK:
            {
                pos = first_end;
                std::string sequence = extract_string(str, first_pos[MOUSE_CLICK], first_end, start_str_arr_lens[MOUSE_CLICK]);

                uint16_t mouse_button = 0xFFFF;
                if(sequence == "L" || sequence == "LEFT")
                    mouse_button = MOUSEEVENTF_LEFTDOWN;
                if(sequence == "R" || sequence == "RIGHT")
                    mouse_button = MOUSEEVENTF_RIGHTDOWN;
                if(sequence == "M" || sequence == "RIGHT")
                    mouse_button = MOUSEEVENTF_MIDDLEDOWN;
                if(mouse_button != 0xFFFF)
                    c->key_vec[key_code].push_back(std::make_unique<MouseClick>(mouse_button));
                else
                    LOGMSG(error, "Invalid mouse button name format!");
                break;
            }
            default:
            {
                LOGMSG(error, "Invalid sequence/text format in line: {}", str.c_str());
                break;
            }
        }
    }
}

void Settings::Init(void)
{
    FILE* file = nullptr;
    if (!std::filesystem::exists("settings.ini"))
    {
        file = fopen("settings.ini", "w");
        if (file == nullptr)
        {
            throw("Shit happend! Press any key to close the application.");
            return;
        }
        fputs("# Settings ini file. Write here the keyboard input keys and the desired outuput. Example:\n", file);
        fputs("# a = Long string which you don't have to type\n# b = uint8_t\n# c = long string\n# This character is only for comments, don't use it afterwards!\n", file);
        fputs("[Keys]\n", file);
        fclose(file);
        file = nullptr;
    }

    boost::property_tree::ptree pt;   /*  KEY_SEQ[CTRL+RSHIFT+A] KEY_SEQ[TAB] KEY_TYPE[Src\Teszt mappa] KEY_SEQ[ESC]  */
    try
    {
        boost::property_tree::ini_parser::read_ini("settings.ini", pt);
    }
    catch(std::exception& e)
    {
        LOGMSG(error, "exception: {}", e.what());
    }

    try
    {
        CustomMacro::Get()->use_per_app_macro = (bool)std::stoi(pt.get_child("Macro_Config").find("UsePerApplicationMacros")->second.data()) != 0;
        CustomMacro::Get()->advanced_key_binding = (bool)std::stoi(pt.get_child("Macro_Config").find("UseAdvancedKeyBinding")->second.data()) != 0;
       
        std::unique_ptr<MacroContainer> p = std::make_unique<MacroContainer>();
        macro_section.clear();
        auto& global_child = pt.get_child("Keys_Global");
        for(auto& key : global_child)
        {
            std::string& str = key.second.data();
            ParseMacroKeys(0, key.first, str, p);
        }
        CustomMacro::Get()->macros.push_back(std::move(p));

        /* load per-application macros */
        size_t counter = 1;
        size_t cnt = 0;
        while((cnt = pt.count("Keys_Macro" + std::to_string(counter))) == 1)
        {
            std::unique_ptr<MacroContainer> p2 = std::make_unique<MacroContainer>();
            auto& ch = pt.get_child("Keys_Macro" + std::to_string(counter));
            for(auto& key : ch)
            {
                if(key.first.data() == std::string("AppName"))
                {
                    p2->name = key.second.data();
                    continue;
                }
                std::string& str = key.second.data();
                ParseMacroKeys(counter, key.first, str, p2);
            }
            counter++;
            CustomMacro::Get()->macros.push_back(std::move(p2));
        }
        CustomMacro::Get()->com_port = std::stoi(pt.get_child("Config").find("COM")->second.data());
        Server::Get()->tcp_port = (uint16_t)std::stoi(pt.get_child("Config").find("TCP_Port")->second.data());
        PrintScreenSaver::Get()->screenshot_key = pt.get_child("Screenshot").find("ScreenshotKey")->second.data();
        PrintScreenSaver::Get()->timestamp_format = pt.get_child("Screenshot").find("ScreenshotDateFormat")->second.data();
        PrintScreenSaver::Get()->screenshot_path = pt.get_child("Screenshot").find("ScreenshotPath")->second.data();
        PathSeparator::Get()->replace_key = pt.get_child("PathSeparator").find("ReplacePathSeparatorKey")->second.data();

        if(!std::filesystem::exists(PrintScreenSaver::Get()->screenshot_path))
            std::filesystem::create_directory(PrintScreenSaver::Get()->screenshot_path);

        DirectoryBackup::Get()->backup_time_format = pt.get_child("BackupSettings").find("BackupFileFormat")->second.data();
        DirectoryBackup::Get()->backup_key = pt.get_child("BackupSettings").find("BackupKey")->second.data();

        /* load backup configs */
        size_t counter_ = 1;
        size_t cnt_ = 0;
        while((cnt_ = pt.count("Backup_" + std::to_string(counter_))) == 1)
        {
            std::string key = "Backup_" + std::to_string(counter_);

            std::filesystem::path from = pt.get_child(key).find("From")->second.data();
            
            std::vector<std::filesystem::path> to;
            boost::split(to, pt.get_child(key).find("To")->second.data(), boost::is_any_of("|"));
           
            std::vector<std::string> ignore_list;
            boost::split(ignore_list, pt.get_child(key).find("Ignore")->second.data(), boost::is_any_of("|"));
            int max_backups = std::stoi(pt.get_child(key).find("MaxBackups")->second.data());
            BackupEntry* b = new BackupEntry(std::move(from), std::move(to), std::move(ignore_list), max_backups);
            
            counter_++;
            DirectoryBackup::Get()->backups.push_back(b);
        }
    }
    catch (boost::property_tree::ini_parser::ini_parser_error &e)
    {
        LOGMSG(error, "exception: {}", e.what());
    }
}
