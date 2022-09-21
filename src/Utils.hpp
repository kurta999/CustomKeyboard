#pragma once

#include <charconv>
#include <string>
#ifdef _WIN32
#include <atlstr.h>
#endif
#include <limits>
#include <random>

#define SAFE_RELEASE(name) \
	if(name) \
		name->Release();

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
    uint16_t crc16_modbus(void* data, size_t len);
#ifndef UNIT_TESTS
    int GetVirtualKeyFromString(const std::string& key);
    std::string GetKeyStringFromVirtualKey(int key_code);

    // !\brief Get folder view 2
    // !\return IFolderView2 pointer
    IFolderView2* GetFolderView2();

    // !\brief Retreives selected items from file explorer
    // !\return Vector of selected items
    std::vector<std::wstring> GetSelectedItemsFromFileExplorer();

    // !\brief Get current directory path from file explorer
    // !\return Current directory path from file explorer
    std::wstring GetDestinationPathFromFileExplorer();


#ifdef _WIN32
    CStringA ExecuteCmdWithoutWindow(const wchar_t* cmd, uint32_t timeout = std::numeric_limits<uint32_t>::min());
#else
    
#endif
    std::string exec(const char* cmd);
#endif

    uint32_t ColorStringToInt(std::string& in);

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
                throw std::runtime_error(std::format("Bad stoi input ({}): {}", from_str, ecode.message()));
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
                throw std::runtime_error(std::format("Bad stoi input ({}): {}", from_str, ecode.message()));
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
                throw std::runtime_error(std::format("Bad stoi input ({}): {}", from_str, ecode.message()));
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

    template<const unsigned num, const char separator>
    void separate(std::string& input)
    {
        for(auto it = input.begin(); (num + 1) <= std::distance(it, input.end()); ++it)
        {
            std::advance(it, num);
            it = input.insert(it, separator);
        }
    }

    template<typename T> T random_mt(T min_val, T max_val)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> distr(min_val, max_val);
        return distr(gen);
    }

}