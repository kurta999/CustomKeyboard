//
// pch.h
//

#pragma once

#include "gtest/gtest.h"

#include <chrono>
#include <stack>

#include <boost/algorithm/string.hpp>

#include "../src/StringToCEscaper.hpp"
#include "../src/StructParser.hpp"
#include "../src/Utils.hpp"

#include "../libs/fmt/format.h"
#include "../libs/fmt/os.h"

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
#define OutputDebugString OutputDebugStringA

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

#define LOGMSG(...)