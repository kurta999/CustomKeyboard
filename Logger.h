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
    Logger() = default;
    void Init(void);
    void Log(severity_level lvl, const char* file, long line, const char* function, const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        char buf[256];
        uint16_t len = vsnprintf(buf, sizeof(buf), (char*)msg, args);
        va_end(args);
        
        time_t current_time;
        tm* current_tm;
        time(&current_time);
        current_tm = localtime(&current_time);
        char date_str[64];
        strftime(date_str, sizeof(date_str), "%Y.%m.%d %H:%M:%S", current_tm);

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
            char fmt[384];
            size_t log_len = snprintf(fmt, sizeof(fmt), "%s [%s:%d - %s] %s", date_str, filename, line, function, msg);
            fwrite(fmt, 1, log_len, fLog);
            fflush(fLog);
        }
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
            frame->log_panel->m_Log->Append(wxString(msg));

    }
private:
    FILE* fLog = nullptr;
};

#ifdef _DEBUG 
#define DBG(str, ...) \
    {\
        char __debug_format_str[256]; \
        snprintf(__debug_format_str, sizeof(__debug_format_str), str, __VA_ARGS__); \
        OutputDebugStringA(__debug_format_str); \
    }
#else
#define DBG(str, ...) 
#endif