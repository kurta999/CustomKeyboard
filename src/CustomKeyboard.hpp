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
    
    // !\brief Helper variable for logger to avoid crash when inserting log messages befor logger frame is created
    bool is_init_finished = false;
};