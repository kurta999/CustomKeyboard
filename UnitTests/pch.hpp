//
// pch.h
//

#pragma once

#define NOMINMAX

#include "gtest/gtest.h"

#include <chrono>
#include <stack>
#include <iostream>
#include <fstream>
#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string_regex.hpp>

enum LogLevel
{
    Verbose,
    Normal,
    Notification,
    Warning,
    Error,
    Critical
};

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

#define LOG(...)
#define LOGW(...)

#include "../src/StringToCEscaper.hpp"
#include "../src/StructParser.hpp"
#include "../src/DirectoryBackup.hpp"
#include "../src/Utils.hpp"

extern "C"
{
#include "../libs/sha256/sha256.h"
}