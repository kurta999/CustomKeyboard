
#include "Settings.h"
#include "Sensors.h"
#include "Logger.h"

#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <cstdint>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <Windows.h>
// [CTRL][RSHIFT][A] [TAB] Src\Teszt mappa [ESC]

void Settings::ParseMacroKeys(size_t id, const char key_code, std::string& str, std::unique_ptr<MacroContainer>& c)
{
    constexpr const char* start_seq_str = "KEY_SEQ[";
    constexpr size_t start_seq_offset_len = std::char_traits<char>::length(start_seq_str);
    constexpr const char* start_text_str = "KEY_TYPE[";
    constexpr size_t start_text_offset_len = std::char_traits<char>::length(start_text_str); 
    constexpr const char* start_delay_str = "DELAY[";
    constexpr size_t start_delay_offset_len = std::char_traits<char>::length(start_delay_str);
    constexpr const char* seq_separator = "+";

    size_t pos = 1;
    while(pos < str.length() - 1)
    {
        size_t first_end = str.find("]", pos + 1);

        size_t first = str.substr(0, first_end).find(start_seq_str, pos - 1);
        size_t first_text = str.substr(0, first_end).find(start_text_str, pos - 1);
        size_t first_delay = str.substr(0, first_end).find(start_delay_str, pos - 1);

        uint8_t input_type = 0xFF;
        /*
        if(first > first_end || first_text >= first_end || first_delay > first_end)
        {
            LOGMSG(error, "Invalid format in settings.ini");
            break;
        }
       */
        if(first != std::string::npos && first_text == std::string::npos && first_delay == std::string::npos)
            input_type = 0;
        if(first == std::string::npos && first_text != std::string::npos && first_delay == std::string::npos)
            input_type = 1;
        if(first == std::string::npos && first_text == std::string::npos && first_delay != std::string::npos)
            input_type = 2;

        if(input_type == 0)
        {
            pos = first_end;
            std::vector<uint16_t> keys;
            std::string sequence = str.substr(first + start_seq_offset_len, first_end - first - start_seq_offset_len);

            boost::char_separator<char> sep(seq_separator);
            boost::tokenizer< boost::char_separator<char> > tok(sequence, sep);
            for(boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
            {
                DBG("Token: %s\n", beg->c_str());
                std::string key_code = *beg;
                uint16_t key = CustomMacro::Get()->GetSpecialKeyCode(key_code);
                keys.push_back(key);
            }

            c->key_vec[key_code].push_back(std::make_unique<KeyCombination>(std::move(keys)));
        }
        else if(input_type == 1)
        {
            pos = first_end;
            std::vector<uint16_t> keys;
            std::string sequence = str.substr(first_text + start_text_offset_len, first_end - first_text - start_text_offset_len);

            DBG("Text Token: %s\n", sequence.c_str());
            for(const auto& i : sequence)
            {
                uint16_t key = CustomMacro::Get()->GetKeyCode(i);
                keys.push_back(key);
            }
            c->key_vec[key_code].push_back(std::make_unique<KeyText>(std::move(keys)));
        }
        else if(input_type == 2)
        {
            pos = first_end;
            std::string sequence = str.substr(first_delay + start_delay_offset_len, first_end - first_delay - start_delay_offset_len);

            uint32_t delay = static_cast<uint32_t>(std::stoi(sequence));
            DBG("Delay: %d\n", delay);
            c->key_vec[key_code].push_back(std::make_unique<KeyDelay>(delay));
        }
        else
        {
            LOGMSG(error, "Invalid sequence/text format in line: %s", str.c_str());
            break;
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
        LOGMSG(error, "exception: %s", e.what());
    }

    try
    {
        CustomMacro::Get()->use_per_app_macro = (bool)std::stoi(pt.get_child("Macro_Config").find("UsePerApplicationMacros")->second.data()) != 0;

        //CustomMacro::Get()->macro_names.push_back("null");

#if 0
        size_t counter = 1;  /* load macro names */
        auto& macro_config = pt.get_child("Macro_Config");
        while(macro_config.find("Macro" + std::to_string(counter)) != macro_config.not_found())
        {
            std::string val = macro_config.find("Macro" + std::to_string(counter))->second.data();
            CustomMacro::Get()->macro_names.push_back(std::move(val));
            counter++;
        }
#endif
       
        std::unique_ptr<MacroContainer> p = std::make_unique<MacroContainer>();
        macro_section.clear();
        auto& global_child = pt.get_child("Keys_Global");
        for(auto& key : global_child)
        {
            std::string& str = key.second.data();
            ParseMacroKeys(0, key.first.c_str()[0], str, p);
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
                ParseMacroKeys(counter, key.first.c_str()[0], str, p2);
            }
            counter++;
            CustomMacro::Get()->macros.push_back(std::move(p2));
        }
        CustomMacro::Get()->com_port = std::stoi(pt.get_child("Config").find("COM")->second.data());
        Sensors::Get()->tcp_port = (uint16_t)std::stoi(pt.get_child("Config").find("TCP_Port")->second.data());
    }
    catch (boost::property_tree::ini_parser::ini_parser_error &e)
    {
        LOGMSG(error, "exception: %s", e.what());
    }


}
