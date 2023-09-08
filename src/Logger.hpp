#pragma once

#include "utils/CSingleton.hpp"

#include <stdarg.h>

#include <wx/wx.h>
#include "gui/MainFrame.hpp"
#include "CustomKeyboard.hpp"
#include <ctime>
#include <fstream>
#include <filesystem>
#include <source_location>

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
    Debug,
    Verbose,
    Normal,
    Notification,
    Warning,
    Error,
    Critical
};

#define WIDEN(quote) WIDEN2(quote)
#define WIDEN2(quote) L##quote

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
    void SetDefaultLogLevel(LogLevel level);
    LogLevel GetDefaultLogLevel() const;
    void SetLogLevelAsString(const std::string& level);
    const std::string GetLogLevelAsString();
    void SetLogFilters(const std::string& filter_list);
    std::string GetLogFilters();
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
        static constexpr std::string_view serverities[] = { "Debug", "Verbose", "Normal", "Notification", "Warning", "Error", "Critical" };
        static constexpr char slash = '/';
        static constexpr char backslash = '\\';
        static constexpr std::string_view gui_str = LOG_GUI_FORMAT;
        static constexpr std::string_view log_str = LOG_FILE_FORMAT;
    };

    template <>
    struct HelperTraits<std::wstring>
    {
        static constexpr std::wstring_view serverities[] = { L"Debug", L"Verbose", L"Normal", L"Notification", L"Warning", L"Error", L"Critical"};
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

    template<class T, typename... Args>
    void LogInternal(LogLevel lvl, const std::source_location& location, std::basic_string_view<T> msg, Args &&...args)
    {
        using string_type = get_fmt_ret_string_type<T>::type;
        typename get_fmt_ret_string_type<T>::type formatted_msg = (sizeof...(args) != 0) ? std::vformat(msg, std::make_format_args<typename get_fmt_mkarg_type<T>::type>(args...)) : msg.data();
        const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        const auto now_truncated_to_ms = std::chrono::floor<std::chrono::milliseconds>(now);
        typename get_fmt_ret_string_type<T>::type str = std::vformat(HelperTraits<string_type>::gui_str, std::make_format_args<typename get_fmt_mkarg_type<T>::type>(now_truncated_to_ms, HelperTraits<string_type>::serverities[lvl], formatted_msg));

        if(lvl < m_DefaultLogLevel)
            return;

        for(auto& i : m_LogFilters)
        {
            if(i.empty()) continue;
            if(std::search(msg.begin(), msg.end(), i.begin(), i.end()) != msg.end())
            {
                return;
            }
        }

        std::unique_lock lock(m_mutex);

        const char* file_name_only = nullptr;
        const char* filename = location.file_name();  /* get filename from file path - __FILE__ macro gives abosulte path for filename */
        for(int i = strlen_helper(location.file_name()); i > 0; i--)
        {
            if(filename[i] == HelperTraits<string_type>::slash || filename[i] == HelperTraits<string_type>::backslash)
            {
                file_name_only = &filename[i + 1];
                break;
            }
        }
        
        if(lvl >= LogLevel::Verbose && lvl <= LogLevel::Critical)
        {
            /* if we're using wide strings */
            if constexpr(std::is_same_v<string_type, std::wstring>)
            {
                std::string str_filename = std::string(file_name_only);
                std::string str_function = std::string(location.function_name());

                std::wstring wfilename_only = std::wstring(str_filename.begin(), str_filename.end());
                std::wstring wfunction_name = std::wstring(str_function.begin(), str_function.end());
                fLog << std::format(HelperTraits<string_type>::log_str, now_truncated_to_ms, HelperTraits<string_type>::serverities[lvl], wfilename_only, location.line(), wfunction_name, formatted_msg);
            }
            else
            {
                fLog << std::format(HelperTraits<string_type>::log_str, now_truncated_to_ms, HelperTraits<string_type>::serverities[lvl], file_name_only, location.line(), location.function_name(), formatted_msg);
            }

            fLog.flush();  /* File operation is handled directly here (at least for now - no time for fully async logger), it's not an expensive operation on modern SSDs */
        }

        preinit_entries.push_back({ wxString(file_name_only), wxString(str) });
    }

    template<typename... Args>
    void LogM(LogLevel lvl, const std::source_location& location, std::string_view msg, Args &&...args)
    {
        LogInternal(lvl, location, msg, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogW(LogLevel lvl, const std::source_location& location, std::wstring_view msg, Args &&...args)
    {
        LogInternal(lvl, location, msg, std::forward<Args>(args)...);
    }

    // !\brief Write given log message to logfile.txt & LogPanel
    // !\param lvl [in] Serverity level
    // !\param file [in] Filename where the call comes from
    // !\param line [in] Line in source file where the call comes from
    // !\param function [in] Function name in source file where the call comes from
    // !\param msg [in] Message to log
    // !\param args [in] va_args arguments for std::format
    template<typename F = const char*, typename G = const char*, class T, typename... Args>
    void Log(LogLevel lvl, const std::source_location& location, T msg, Args &&...args)
    {
        using X = std::decay_t<decltype(msg)>;
        if constexpr(std::is_same_v<X, const char*>)
        {
            LogM(lvl, location, msg, std::forward<Args>(args)...);
        }
        else if constexpr(std::is_same_v<X, const wchar_t*>)
        {
            LogW(lvl, location, msg, std::forward<Args>(args)...);
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
    if(lvl > LogLevel::Debug && lvl <= LogLevel::Critical)
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
    void AppendPreinitedEntries();

    // !\brief Tick funciton
    void Tick();

private:
    LogLevel StringToLogLevel(const std::string& level);

    std::string LogLevelToString(LogLevel level);

    // !\brief Default log level
    LogLevel m_DefaultLogLevel = LogLevel::Verbose;

    // !\brief File handle for log 
    std::ofstream fLog;

    // !\brief Preinited log messages
    std::vector<LogEntry> preinit_entries;

    // !\brief Pointer to LogPanel
    ILogHelper* m_helper = nullptr;

    // !\brief Log filters
    std::vector<wxString> m_LogFilters;

    // !\brief Logger's mutex
    std::mutex m_mutex;
};

/* Global LOG function */
template <typename... Ts>
struct LOG
{
    LOG(LogLevel lvl, Ts&&... ts, const std::source_location& loc = std::source_location::current())
    {
        Logger::Get()->Log(lvl, loc, std::forward<Ts>(ts)...);
    }
    ~LOG() = default;
};

template <typename... Ts>
LOG(LogLevel lvl, Ts&&...) -> LOG<Ts...>;