#pragma once

#include <charconv>

namespace utils
{
    bool is_number(const std::string& s);
    void MadeTextFromKeys(std::string& chr);
    std::string GetDataUnit(size_t input);

    template <typename R, typename S> inline R stoi(const S& from_str)
    {
        /*if constexpr(std::is_integral_v<R>)
            static_assert(false, "R is not integral!");*/

        using T = std::decay_t<decltype(from_str)>;
        if constexpr(std::is_same_v<T, std::string>)
        {
            std::string_view conv = from_str;
            auto int_val = 0;
            auto ret = std::from_chars(conv.data(), conv.data() + conv.size(), int_val);
            if(ret.ec != std::errc())
                throw std::runtime_error(fmt::format("Bad input: {}", from_str));
            return int_val;
        }
        else if constexpr(std::is_same_v<T, const char*>)
        {
            size_t len = strlen(from_str);
            decltype(auto) int_val = 0;
            auto ret = std::from_chars(from_str, from_str + len, int_val);
            if(ret.ec != std::errc())
                throw std::runtime_error(fmt::format("Bad input: {}", from_str));
            return int_val;
        }
        else if constexpr(std::is_same_v<T, char*>)
        {
            size_t len = strlen(from_str);
            decltype(auto) int_val = 0;
            auto ret = std::from_chars(from_str, from_str + len, int_val);
            if(ret.ec != std::errc())
                throw std::runtime_error(fmt::format("Bad input: {}", from_str));
            return int_val;
        }
        else
            static_assert(false, "bad type - from_str!");
    }

    template <typename S> inline bool stob(const S& from_str)
    {
        /*if constexpr(std::is_integral_v<R>)
            static_assert(false, "R is not integral!");*/

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
            static_assert(false, "bad type - from_str!");
    }
}