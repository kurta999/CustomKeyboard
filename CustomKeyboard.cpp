#include <boost/asio.hpp>

#include "CustomKeyboard.h"
#include "main_frame.h"
#include <wx/wxprec.h>
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <string>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/taskbar.h"

//#include <Windows.h>

#include "CustomMacro.h"
#include "Logger.h"
#include "Settings.h"
#include "Server.h"
#include "Database.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    Logger::Get()->Init();
    Settings::Get()->Init();
    CustomMacro::Get()->Init();
    Server::Get()->Init();
    Database::Get()->Init();

    LOGMSG(normal, "Starting...");
    LOGMSG(warning, "Testing log file");
    if (!wxTaskBarIcon::IsAvailable())
        LOGMSG(normal, "There appears to be no system tray support in your current environment. This app may not behave as expected.");
    MyFrame* frame = new MyFrame(wxT("wxCreator"));
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}
/*
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{

}*/