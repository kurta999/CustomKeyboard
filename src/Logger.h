#pragma once

#include "utils/CSingleton.h"

#include <stdarg.h>

#include <wx/wx.h>
#include "gui/main_frame.h"
#include "CustomKeyboard.h"
#include <ctime>
#include <filesystem>
#include <fmt/format.h>
#include "fmt/chrono.h"

DECLARE_APP(MyApp);

enum severity_level
{
    normal,
    notification,
    warning,
    error,
    critical
};

#define LOGMSG(level, message, ...)      \
    do {\
    Logger::Get()->Log(level, __FILE__, __LINE__, __FUNCTION__, message, __VA_ARGS__); \
} while(0)

class Logger : public CSingleton < Logger >
{
    friend class CSingleton < Logger >;
public:
    Logger();
    ~Logger();

    template<typename... Args>
    void Log(severity_level lvl, const char* file, long line, const char* function, const char* msg, Args &&...args)
    {
        std::string str;
        std::string formatted_msg = (sizeof...(args) != 0) ? fmt::format(msg, std::forward<Args>(args)...) : msg;
        time_t current_time;
        tm* current_tm;
        time(&current_time);
        current_tm = localtime(&current_time);
        str = fmt::format("{:%Y.%m.%d %H:%M:%S} [{}] {}", *current_tm, serverity_str[lvl], formatted_msg);
#if DEBUG
        OutputDebugStringA(str.c_str());
        OutputDebugStringA("\n");
#endif
        const char* filename = file;
        for(int i = strlen(file); i > 0; i--)
        {
            if(file[i] == '/' || file[i] == '\\')
            {
                filename = &file[i + 1];
                    break;
            }
        }
        if(lvl > normal)
        {
            std::string str_file = fmt::format("{:%Y.%m.%d %H:%M:%S} [{}] [{}:{} - {}] {}\n", *current_tm, serverity_str[lvl], filename, line, function, formatted_msg);
            fwrite(str_file.c_str(), 1, str_file.length(), fLog);
            fflush(fLog);
        }
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame && frame->log_panel)
            frame->log_panel->m_Log->Append(wxString(str));
        else
            preinit_entries.Add(str);
    }

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
    FILE* fLog = nullptr;
    wxArrayString preinit_entries;

    static inline const char* serverity_str[] = { "Normal", "Notification", "Warning", "Error", "Critical" };
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