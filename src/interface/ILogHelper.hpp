#pragma once

#include <string>

class ILogHelper
{
public:
    virtual void ClearEntries() = 0;
    virtual void AppendLog(std::string& line) = 0;
};
