#pragma once

#include "resource.h"

#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
};