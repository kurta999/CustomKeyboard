#pragma once

#ifdef _WIN32
#include "resource.h"
#endif

#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    int OnExit() override;
    void OnUnhandledException() override;
};