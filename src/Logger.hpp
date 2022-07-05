#pragma once

#include "utils/CSingleton.hpp"

#include <stdarg.h>

#include <wx/wx.h>
#include "gui/MainFrame.hpp"
#include "CustomKeyboard.hpp"
#include <ctime>
#include <fstream>
#include <filesystem>
#ifndef _WIN32
    #ifndef FMT_HEADER_ONLY
        #define FMT_HEADER_ONLY
    #endif
#endif
#include <fmt/format.h>
#include "fmt/chrono.h"

DECLARE_APP(MyApp);

enum LogLevel
{
    Normal,
    Notification,
    Warning,
    Error,
    Critical
};

#define LOG(level, message, ...)      \
    do {\
    Logger::Get()->Log(level, __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__); \
} while(0)

class Logger : public CSingleton < Logger >
{
    friend class CSingleton < Logger >;
public:
    Logger();
    ~Logger() = default;

    // !\brief Write given log message to logfile.txt & LogPanel
    // !\param lvl [in] Serverity level
    // !\param file [in] Filename where the call comes from
    // !\param line [in] Line in source file where the call comes from
    // !\param function [in] Function name in source file where the call comes from
    // !\param msg [in] Message to log
    // !\param args [in] va_args arguments for fmt::format
    template<typename... Args>
    void Log(LogLevel lvl, const char* file, long line, const char* function, const char* msg, Args &&...args)
    {
        std::string str;
        std::string formatted_msg = (sizeof...(args) != 0) ? fmt::format(msg, std::forward<Args>(args)...) : msg;
        time_t current_time;
        tm* current_tm;
        time(&current_time);
        current_tm = localtime(&current_time);
        str = fmt::format("{:%Y.%m.%d %H:%M:%S} [{}] {}", *current_tm, severity_str[lvl], formatted_msg);
#if DEBUG
        OutputDebugStringA(str.c_str());
        OutputDebugStringA("\n");
#endif
        const char* filename = file;  /* get filename from file path - __FILE__ macro gives abosulte path for filename */
        for(int i = strlen(file); i > 0; i--)
        {
            if(file[i] == '/' || file[i] == '\\')
            {
                filename = &file[i + 1];
                    break;
            }
        }
        if(lvl > LogLevel::Normal && lvl <= LogLevel::Critical)
        {
            fLog << fmt::format("{:%Y.%m.%d %H:%M:%S} [{}] [{}:{} - {}] {}\n", *current_tm, severity_str[lvl], filename, line, function, formatted_msg);
            fLog.flush();
        }
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(wxGetApp().is_init_finished && frame && frame->log_panel && frame->log_panel->m_Log)
        {
            frame->log_panel->m_Log->Append(wxString(str));
            frame->log_panel->m_Log->ScrollLines(frame->log_panel->m_Log->GetCount());
        }
        else
            preinit_entries.Add(str);
    }

    // !\brief Append log messages to log panel which were logged before log panel was constructor
    void AppendPreinitedEntries()
    {
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame && frame->log_panel)
        {
            frame->log_panel->m_Log->Append(preinit_entries);
            preinit_entries.Clear();
        }
    }
private:
    // !\brief File handle for log 
    std::ofstream fLog;

    // !\brief Preinited log messages
    wxArrayString preinit_entries;

    // !\brief Serverity level in string format
    static inline const char* severity_str[] = { "Normal", "Notification", "Warning", "Error", "Critical" };
};

#ifdef _DEBUG /* this is only for debugging, it remains oldschool */
#define DBG(str, ...) \
    {\
        char __debug_format_str[64]; \
        snprintf(__debug_format_str, sizeof(__debug_format_str), str, __VA_ARGS__); \
        OutputDebugStringA(__debug_format_str); \
    }

#define DBGW(str, ...) \
    {\
        wchar_t __debug_format_str[128]; \
        wsprintfW(__debug_format_str, str, __VA_ARGS__); \
        OutputDebugStringW(__debug_format_str); \
    }
#else
#define DBG(str, ...)
#define DBGW(str, ...)
#endif