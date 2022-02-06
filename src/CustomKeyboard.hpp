#pragma once

#include "resource.h"

#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    int OnExit() override;
    void OnUnhandledException() override;
};