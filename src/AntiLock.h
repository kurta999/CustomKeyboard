#pragma once

#include "utils/CSingleton.h"

class AntiLock : public CSingleton < AntiLock >
{
    friend class CSingleton < AntiLock >;

public:
    AntiLock() = default;
    ~AntiLock() = default;

    void Process();

    bool is_enabled = false;
    uint32_t timeout = 300; /* Timeout [s] */
private:
    bool step_forward = false;
};