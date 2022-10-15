#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <chrono>

typedef enum : uint8_t
{
    WINDOWS_TERMINAL,
    COMMAND_LINE,
    POWER_SHELL,
    BASH_TERMINAL,
} TerminalType;

class TerminalHotkey : public CSingleton < TerminalHotkey >
{
    friend class CSingleton < TerminalHotkey >;

public:
    TerminalHotkey() = default;

    // !\brief Is enabled?
    bool is_enabled = true;

    // !\brief Type of terminal which one to open
    TerminalType type{TerminalType::WINDOWS_TERMINAL};

    // !\brief Set key
    // !\param key_str [in] Key as string
    void SetKey(const std::string& key_str);

    // !\brief Return key as string
    std::string GetKey();

    // !\brief Updates hotkey registration in main frame
    void UpdateHotkeyRegistration();

    // !\brief Process function
    void Process();

private:
    // !\brief VK key code for trigger key
#ifdef _WIN32
    int vkey = VK_F7;
#else
    int vkey = 0;
#endif
    // !\brief Open terminal with given path
    // !\param path [in] Path where to open the terminal
    void OpenTerminal(std::wstring& path);
};