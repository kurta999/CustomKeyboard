#pragma once

#include "utils/CSingleton.hpp"

#include <stdarg.h>

#include <wx/wx.h>
#include "gui/MainFrame.hpp"
#include "CustomKeyboard.hpp"
#include <ctime>
#include <fstream>
#include <filesystem>

#include "ILogHelper.hpp"

#ifndef _WIN32
#include <fmt/format.h>
#include <fmt/chrono.h>
#endif

template<class> inline constexpr bool always_false_v = false;

DECLARE_APP(MyApp);

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

enum LogLevel
{
    Verbose,
    Normal,
    Notification,
    Warning,
    Error,
    Critical
};

#define WIDEN(quote) WIDEN2(quote)
#define WIDEN2(quote) L##quote

#define LOG(level, message, ...) \
    do {\
    {Logger::Get()->Log(level, __FILE__, __LINE__, __FUNCTION__, message, ##__VA_ARGS__);} \
} while(0)

#ifdef _WIN32
#define LOGW(level, message, ...) \
    do {\
    {Logger::Get()->Log(level, WIDEN(__FILE__), __LINE__, WIDEN(__FUNCTION__), message, ##__VA_ARGS__);} \
} while(0)
#else
#define LOGW(str, ...)
#endif

class LogEntry
{
public:
    LogEntry(wxString file_, wxString message_) :
        file(file_), message(message_)
    {

    }

    wxString file;
    wxString message;
};

class Logger : public CSingleton < Logger >
{
    friend class CSingleton < Logger >;
public:
    Logger();
    ~Logger() = default;
    void SetLogHelper(ILogHelper* helper);
    bool SearchInLogFile(std::string_view filter, std::string_view log_level);

#ifdef _WIN32  /* std::format version with both std::string & std::wstring support - GCC's std::format and std::chrono::current_zone implementation is still missing - 2022.10.28 */
#define LOG_GUI_FORMAT "{:%Y.%m.%d %H:%M:%S} [{}] {}"
#define LOG_FILE_FORMAT "{:%Y.%m.%d %H:%M:%S} [{}] [{}:{} - {}] {}\n"

    template <class T>
    struct HelperTraits
    {
        static_assert(always_false_v<T>, "Invalid type. Only std::string and std::wstring are accepted!");
    };

    template <>
    struct HelperTraits<std::string>
    {
        static constexpr std::string_view serverities[] = { "Verbose", "Normal", "Notification", "Warning", "Error", "Critical" };
        static constexpr char slash = '/';
        static constexpr char backslash = '\\';
        static constexpr std::string_view gui_str = LOG_GUI_FORMAT;
        static constexpr std::string_view log_str = LOG_FILE_FORMAT;
    };

    template <>
    struct HelperTraits<std::wstring>
    {
        static constexpr std::wstring_view serverities[] = { L"Verbose", L"Normal", L"Notification", L"Warning", L"Error", L"Critical" };
        static constexpr wchar_t slash = L'/';
        static constexpr wchar_t backslash = L'\\';
        static constexpr std::wstring_view gui_str = WIDEN(LOG_GUI_FORMAT);
        static constexpr std::wstring_view log_str = WIDEN(LOG_FILE_FORMAT);
    };

    template<typename T>
    size_t strlen_helper(T input)
    {
        using X = std::decay_t<decltype(input)>;
        if constexpr(std::is_same_v<X, const char*>)
        {
            return strlen(input);
        }
        else if constexpr(std::is_same_v<X, const wchar_t*>)
        {
            return std::wstring(input).length();
        }
        else
        {
            static_assert(always_false_v<X>, "Invalid type. Only const char* and const wchar_t* are accepted!");
        }
    }

    template <typename T> struct get_fmt_mkarg_type;
    template <> struct get_fmt_mkarg_type<const wchar_t*> { using type = std::wformat_context; };
    template <> struct get_fmt_mkarg_type<const wchar_t> { using type = std::wformat_context; };
    template <> struct get_fmt_mkarg_type<wchar_t> { using type = std::wformat_context; };
    template <> struct get_fmt_mkarg_type<const char*> { using type = std::format_context; };
    template <> struct get_fmt_mkarg_type<const char> { using type = std::format_context; };
    template <> struct get_fmt_mkarg_type<char> { using type = std::format_context; };

    template <typename T> struct get_fmt_ret_string_type;
    template <> struct get_fmt_ret_string_type<const wchar_t*> { using type = std::wstring; };
    template <> struct get_fmt_ret_string_type<const wchar_t> { using type = std::wstring; };
    template <> struct get_fmt_ret_string_type<wchar_t> { using type = std::wstring; };
    template <> struct get_fmt_ret_string_type<const char*> { using type = std::string; };
    template <> struct get_fmt_ret_string_type<const char> { using type = std::string; };
    template <> struct get_fmt_ret_string_type<char> { using type = std::string; };

    template<typename F = const char*, typename G = const char*, class T, typename... Args>
    void LogInternal(LogLevel lvl, F file, long line, G function, std::basic_string_view<T> msg, Args &&...args)
    {
        using string_type = get_fmt_ret_string_type<T>::type;
        typename get_fmt_ret_string_type<T>::type formatted_msg = (sizeof...(args) != 0) ? std::vformat(msg, std::make_format_args<typename get_fmt_mkarg_type<T>::type>(args...)) : msg.data();
        const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        const auto now_truncated_to_ms = std::chrono::floor<std::chrono::milliseconds>(now);
        typename get_fmt_ret_string_type<T>::type str = std::vformat(HelperTraits<string_type>::gui_str, std::make_format_args<typename get_fmt_mkarg_type<T>::type>(now_truncated_to_ms, HelperTraits<string_type>::serverities[lvl], formatted_msg));

        F filename = file;  /* get filename from file path - __FILE__ macro gives abosulte path for filename */
        for(int i = strlen_helper(file); i > 0; i--)
        {
            if(file[i] == HelperTraits<string_type>::slash || file[i] == HelperTraits<string_type>::backslash)
            {
                filename = &file[i + 1];
                break;
            }
        }

        if(lvl >= LogLevel::Verbose && lvl <= LogLevel::Critical)
        {
            fLog << std::format(HelperTraits<string_type>::log_str, now_truncated_to_ms, HelperTraits<string_type>::serverities[lvl], filename, line, function, formatted_msg);
            fLog.flush();
        }

        bool ret = m_mutex.try_lock_for(std::chrono::milliseconds(10));
        if(ret)
        {
            MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
            if(wxGetApp().is_init_finished && frame && frame->log_panel && frame->log_panel->m_Log && m_helper)
            {
                m_helper->AppendLog(file, str, true);
            }
            else
                preinit_entries.push_back({ wxString(file), wxString(str) });
        }

        if(ret)
            m_mutex.unlock();
        else
            preinit_entries.push_back({ wxString(file), wxString(str) });
    }

    template<typename F = const char*, typename G = const char*, typename... Args>
    void LogM(LogLevel lvl, F file, long line, G function, std::string_view msg, Args &&...args)
    {
        LogInternal(lvl, file, line, function, msg, std::forward<Args>(args)...);
    }

    template<typename F = const char*, typename G = const char*, typename... Args>
    void LogW(LogLevel lvl, F file, long line, G function, std::wstring_view msg, Args &&...args)
    {
        LogInternal(lvl, file, line, function, msg, std::forward<Args>(args)...);
    }

    // !\brief Write given log message to logfile.txt & LogPanel
    // !\param lvl [in] Serverity level
    // !\param file [in] Filename where the call comes from
    // !\param line [in] Line in source file where the call comes from
    // !\param function [in] Function name in source file where the call comes from
    // !\param msg [in] Message to log
    // !\param args [in] va_args arguments for std::format
    template<typename F = const char*, typename G = const char*, class T, typename... Args>
    void Log(LogLevel lvl, F file, long line, G function, T msg, Args &&...args)
    {
        using X = std::decay_t<decltype(msg)>;
        if constexpr(std::is_same_v<X, const char*>)
        {
            LogM(lvl, file, line, function, msg, std::forward<Args>(args)...);
        }
        else if constexpr(std::is_same_v<X, const wchar_t*>)
        {
            LogW(lvl, file, line, function, msg, std::forward<Args>(args)...);
        }
        else
        {
            static_assert(always_false_v<X>, "Invalid type. Only const char* and const wchar_t* are accepted!");
        }
    }
#else
static constexpr std::string_view severity_str[] = { "Verbose", "Normal", "Notification", "Warning", "Error", "Critical" };

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
    if(lvl > LogLevel::Verbose && lvl <= LogLevel::Critical)
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
        preinit_entries.push_back({ wxString(file), wxString(str) });
}

#endif

    // !\brief Append log messages to log panel which were logged before log panel was constructor
    void AppendPreinitedEntries()
    {
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame && frame->log_panel && m_helper)
        {
            bool ret = m_mutex.try_lock_for(std::chrono::milliseconds(1000));
            if(ret)
            {
                for(auto& i : preinit_entries)
                {
                    if(!i.message.empty())
                        m_helper->AppendLog(i.file.ToStdString(), i.message.ToStdString(), false);
                }
                preinit_entries.clear();
                m_mutex.unlock();
            }
        }
    }

private:
    // !\brief File handle for log 
    std::ofstream fLog;

    // !\brief Preinited log messages
    std::vector<LogEntry> preinit_entries;

    // !\brief Pointer to LogPanel
    ILogHelper* m_helper = nullptr;

    // !\brief Logger's mutex
    std::timed_mutex m_mutex;
};