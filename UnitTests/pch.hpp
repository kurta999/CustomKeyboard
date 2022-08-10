//
// pch.h
//

#pragma once

#define NOMINMAX

#include "gtest/gtest.h"

#include <chrono>
#include <stack>
#include <iostream>
#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>

#include "../src/StringToCEscaper.hpp"
#include "../src/StructParser.hpp"
#include "../src/Utils.hpp"

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