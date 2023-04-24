#pragma once

#include <string>

class ILogHelper
{
public:
    virtual ~ILogHelper() { }

    virtual void ClearEntries() = 0;
    virtual void AppendLog(const std::string& file, const std::string& line, bool scroll_to_end = false) = 0;
    virtual void AppendLog(const std::wstring& file, const std::wstring& line, bool scroll_to_end = false) = 0;
};
