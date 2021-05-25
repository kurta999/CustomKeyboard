#pragma once

#include "utils/CSingleton.h"

#include <stdarg.h>

#include <wx/wx.h>
/*
#include <wx/slider.h>
#include <wx/artprov.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/timectrl.h>
#include <wx/calctrl.h>
#include <wx/treectrl.h>
*/
#include "main_frame.h"
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
        std::string formatted_msg = fmt::format(msg, std::forward<Args>(args)...);
        time_t current_time;
        tm* current_tm;
        time(&current_time);
        current_tm = localtime(&current_time);
        str = fmt::format("{:%Y.%m.%d %H:%M:%S} {}", *current_tm, formatted_msg);

        //OutputDebugStringA(buf);
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
            std::string str_file = fmt::format("{:%Y.%m.%d %H:%M:%S} [{}:{} - {}] {}", *current_tm, filename, line, function, formatted_msg);
            fwrite(str_file.c_str(), 1, str_file.length(), fLog);
            fflush(fLog);
        }
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
            frame->log_panel->m_Log->Append(wxString(str));
    }
private:
    FILE* fLog = nullptr;
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