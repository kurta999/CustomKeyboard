#pragma once

#include "utils/CSingleton.h"
#include "CustomMacro.h"

class Settings : public CSingleton < Settings >
{
    friend class CSingleton < Settings >;

public:
    Settings() = default;
    void Init(void);
private:
};