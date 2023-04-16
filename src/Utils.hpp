#pragma once

#include <charconv>
#include <string>
#ifdef _WIN32
#include <atlstr.h>
#endif
#include <limits>
#include <random>
#include <span>
#include <thread>
#include <boost/algorithm/hex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#ifndef UNIT_TESTS
#include "Logger.hpp"
#endif

#define SAFE_RELEASE(name) \
	if(name) \
		name->Release();

/* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
#define RGB_TO_WXCOLOR(color) \
    wxColour(boost::endian::endian_reverse(color << 8))

#define WXCOLOR_TO_RGB(color) \
    boost::endian::endian_reverse(color << 8)

namespace utils
{
    template<class> inline constexpr bool always_false_v = false;

    void SetThreadName(std::thread& thread, const char* threadName);
    void SetThreadName(std::jthread& thread, const char* threadName);

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

#ifdef _WIN32
    // !\brief Get folder view 2
    // !\return IFolderView2 pointer
    IFolderView2* GetFolderView2();
#endif

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
    void ConvertHexBufferToString(const std::vector<uint8_t>& in, std::string& out);
    void ConvertHexBufferToString(const char* in, size_t len, std::string& out);

    template<typename T, std::size_t length>
    void ConvertHexStringToBuffer(const std::string& in, std::span<T, length> out)
    {
        std::string hash;
        try
        {
            hash = boost::algorithm::unhex(in);
        }
        catch(...)
        {
            LOG(LogLevel::Error, "Exception with boost::algorithm::unhex, str: {}", in);
        }
        if(hash.size() < out.size_bytes())
            std::copy(hash.begin(), hash.end(), out.data());
        else
        {
            LOG(LogLevel::Error, "Output buffer size is smaller than required! hash size: {}, out size: {}", out.size_bytes(), hash.size());
        }
    }

    uint32_t ColorStringToInt(const std::string& in);
    const std::string ColorIntToString(uint32_t in);

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
            if(from_str == nullptr)
                return false;

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

#ifndef _WIN32
    uint32_t GetTickCount();
#endif

    constexpr unsigned long RGB_TO_INT(int r, int g, int b)
    {
        return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
    }
}

namespace utils
{
    namespace xml
    {
        template <typename T, typename U> void ReadChildIfexists(const boost::property_tree::ptree::value_type& v, const std::string& child_name, U& out)
        {
            auto child_val = v.second.get_child_optional(child_name);
            if(child_val)
                out = child_val->get_value<T>();
        }

        template <typename A, typename U> void ReadValueIfexists(A v, const std::string& child_name, U& out)
        {
            auto child_val = v->get_optional<U>(child_name);
            if(child_val)
                out = *child_val;
        }
    }

    namespace ini = xml;
    namespace ptree = xml;
}

namespace utils
{
    template <std::size_t N>
    struct type_of_size
    {
        typedef char type[N];
    };
    struct type_of_size_nonarray
    {
        typedef char type;
    };

    template <typename T, std::size_t Size>
    typename type_of_size<Size>::type& sizeof_array_helper(T(&)[Size]);
    template <typename T>
    typename type_of_size_nonarray::type& sizeof_array_helper(T&);
}

#define pp_sizeof_array(pArray) sizeof(utils::sizeof_array_helper(pArray))
