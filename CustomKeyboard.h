#pragma once

#include "resource.h"

#include <wx/wx.h>

enum
{
    kRowId,
    kSensorId,
    kTemp,
    kHum,
    kCo2,
    kVoc,
    kPm25,
    kPm10,
    kLux,
    kCct,
    kTime
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};