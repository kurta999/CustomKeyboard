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

#include "CustomMacro.h"
#include "Logger.h"
#include "Settings.h"
#include "Server.h"
#include "Database.h"
#include "Sensors.h"
#include "StructParser.h"
#include "PrintScreenSaver.h"
#include "DirectoryBackup.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    Logger::Get()->Init();
    Settings::Get()->Init();
    CustomMacro::Get()->Init();
    Server::Get()->Init();
    Sensors::Get()->Init();
    Database::Get()->Init();
    StructParser::Get()->Init();
    PrintScreenSaver::Get()->Init();
    DirectoryBackup::Get()->Init();

    if (!wxTaskBarIcon::IsAvailable())
        LOGMSG(normal, "There appears to be no system tray support in your current environment. This app may not behave as expected.");
    MyFrame* frame = new MyFrame(wxT("CustomKeyboard"));
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}