#pragma once

#include <string>

class ICanResultPanel
{
public:
    virtual ~ICanResultPanel() {}

    virtual void AddToLog(std::string str) = 0;
};