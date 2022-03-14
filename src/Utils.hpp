#pragma once

#include <charconv>
#include <string>

namespace utils
{
    template<class> inline constexpr bool always_false_v = false;

    inline std::string extract_string(std::string& str, size_t start, size_t start_end, size_t len)
    {
        return str.substr(start + len, start_end - start - len);
    }

    bool is_number(const std::string& s);
    void MadeTextFromKeys(std::string& chr);
    std::string GetDataUnit(size_t input);
    size_t MBStringToWString(const std::string& src, std::wstring& dest);
    size_t WStringToMBString(const std::wstring& src, std::string& dest);
    wxKeyCode GetVirtualKeyFromString(const std::string& key);

    template <typename R, typename S> inline R stoi(const S& from_str)
    {
        if constexpr(!std::is_arithmetic_v<R>)
            static_assert(always_false_v<R>, "R is not arithmetic!");

        using T = std::decay_t<decltype(from_str)>;
        if constexpr(std::is_same_v<T, std::string>)
        {
            std::string_view conv = from_str;
            auto int_val = 0;
            auto ret = std::from_chars(conv.data(), conv.data() + conv.size(), int_val);
            if(ret.ec != std::errc())
            {
                std::error_code ecode = std::make_error_code(ret.ec);
                throw std::runtime_error(fmt::format("Bad stoi input ({}): {}", from_str, ecode.message()));
            }
            return int_val;
        }
        else if constexpr(std::is_same_v<T, const char*>)
        {
            size_t len = strlen(from_str);
            decltype(auto) int_val = 0;
            auto ret = std::from_chars(from_str, from_str + len, int_val);
            if(ret.ec != std::errc())
            {
                std::error_code ecode = std::make_error_code(ret.ec);
                throw std::runtime_error(fmt::format("Bad stoi input ({}): {}", from_str, ecode.message()));
            }            
            return int_val;
        }
        else if constexpr(std::is_same_v<T, char*>)
        {
            size_t len = strlen(from_str);
            decltype(auto) int_val = 0;
            auto ret = std::from_chars(from_str, from_str + len, int_val);
            if(ret.ec != std::errc())
            {
                std::error_code ecode = std::make_error_code(ret.ec);
                throw std::runtime_error(fmt::format("Bad stoi input ({}): {}", from_str, ecode.message()));
            }
            return int_val;
        }
        else
            static_assert(always_false_v<T>, "bad type - from_str!");
    }

    template <typename S> inline bool stob(const S& from_str)
    {
        using T = std::decay_t<decltype(from_str)>;
        if constexpr(std::is_same_v<T, std::string>)
        {
            return !(from_str == "false" || from_str[0] == '0');
        }
        else if constexpr(std::is_same_v<T, const char*>)
        {
            return !(!strcmp(from_str, "false") || from_str[0] == '0');
        }
        else
            static_assert(always_false_v<T>, "bad type - from_str!");
    }
}