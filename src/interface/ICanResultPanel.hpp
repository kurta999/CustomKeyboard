#pragma once

#include <string>

class ICanResultPanel
{
public:
    virtual ~ICanResultPanel() = default;

    virtual void AddToLog(std::string str) = 0;
};