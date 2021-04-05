
#include "Settings.h"
#include "Sensors.h"
#include "Logger.h"

#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <cstdint>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include <Windows.h>
// [CTRL][RSHIFT][A] [TAB] Src\Teszt mappa [ESC]


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

    constexpr const char* start_seq_str = "KEY_SEQ[";
    constexpr size_t start_seq_offset_len = std::char_traits<char>::length(start_seq_str);
    constexpr const char* start_text_str = "KEY_TYPE[";
    constexpr size_t start_text_offset_len = std::char_traits<char>::length(start_text_str);
    constexpr const char* seq_separator = "+";

    DBG("My message %d %f %s\n", 5, 15.3f, "teszt string");
    /*
                    size_t first = pos = str.find_first_of(start_seq_str, pos);
                size_t second = str.find_first_of("]", pos);

                size_t first_text = pos = str.find_first_of(start_seq_str, pos);
                size_t second_text = str.find_first_of("]", pos);

                */
    /*  KEY_SEQ[CTRL+RSHIFT+A] KEY_SEQ[TAB] KEY_TYPE[Src\Teszt mappa] KEY_SEQ[ESC]  */
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("settings.ini", pt);
    try
    {
        auto &sec = pt.get_child("Keys");
        for (auto& key : sec)
        {
            //CustomMacro::Get()->key_bindings[key.first.c_str()[0]] = key.second.data();

            std::string &str = key.second.data();
            size_t pos = 1;
            while (pos < str.length() - 1)
            {
                size_t first = str.find(start_seq_str, pos - 1);
                size_t first_text = str.find(start_text_str, pos -1 );

                size_t second = first < first_text ? str.find("]", first) : std::string::npos;
                size_t second_text = first_text < first ? str.find("]", first_text) : std::string::npos;

                if (first != std::string::npos && second != std::string::npos)
                {
                    pos = second;
                    std::vector<uint16_t> keys;
                    std::string sequence = str.substr(first + start_seq_offset_len, second - first - start_seq_offset_len);

                    boost::char_separator<char> sep(seq_separator);
                    boost::tokenizer< boost::char_separator<char> > tok(sequence, sep);
                    for (boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
                    {
                        char logstr[256];
                        snprintf(logstr, sizeof(logstr), "Token: %s\n", beg->c_str());
                        OutputDebugStringA(logstr);

                        std::string key_code = *beg;
                        uint16_t key = CustomMacro::Get()->GetSpecialKeyCode(key_code);
                        keys.push_back(key);
                    }

                    const char key_code = key.first.c_str()[0];
                    CustomMacro::Get()->key_map[key_code].push_back(new KeyCombination(std::move(keys))); // TODO: remove this ugly one
                }
                else if(first_text != std::string::npos && second_text != std::string::npos)
                {
                    pos = second_text;
                    std::vector<uint16_t> keys;
                    std::string sequence = str.substr(first_text + start_text_offset_len, second_text - first_text - start_text_offset_len);

                    boost::char_separator<char> sep(seq_separator);
                    boost::tokenizer< boost::char_separator<char> > tok(sequence, sep);
                    for(boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
                    {
                        char logstr[256];
                        snprintf(logstr, sizeof(logstr), "TToken: %s\n", beg->c_str());
                        OutputDebugStringA(logstr);

                        std::string key_code = *beg;
                        for(const auto& i : key_code)
                        {
                            uint16_t key = CustomMacro::Get()->GetKeyCode(i);
                            keys.push_back(key);
                        }

                    }

                    const char key_code = key.first.c_str()[0];
                    CustomMacro::Get()->key_map[key_code].push_back(new KeyText(std::move(keys))); // TODO: remove this ugly one

                }
                else
                {
                    LOGMSG(error, "Invalid sequence/text format in line: %s", str.c_str());
                    break;
                }
            }
        }

        CustomMacro::Get()->com_port = std::stoi(pt.get_child("Config").find("COM")->second.data());
        Sensors::Get()->tcp_port = (uint16_t)std::stoi(pt.get_child("Config").find("TCP_Port")->second.data());
    }
    catch (boost::property_tree::ini_parser::ini_parser_error &e)
    {
        LOGMSG(error, "exception: %s", e.what());
    }


}
