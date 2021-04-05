#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"

#include <inttypes.h>
#include <map>
#include <string>


class Sensors : public CSingleton < Sensors >
{
    friend class CSingleton < Sensors >;

public:
    Sensors() = default;
    void Init(void);

private:
    friend class Settings;
    uint16_t tcp_port = 0;
};