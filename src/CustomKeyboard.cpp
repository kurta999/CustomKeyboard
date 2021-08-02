#include "pch.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    Settings::Get()->Init();
    CustomMacro::Get()->Init();
    Server::Get()->Init();
    Sensors::Get()->Init();
    StructParser::Get()->Init();
    PrintScreenSaver::Get()->Init();
    DirectoryBackup::Get()->Init();

    if (!wxTaskBarIcon::IsAvailable())
        LOGMSG(normal, "There appears to be no system tray support in your current environment. This app may not behave as expected.");
    MyFrame* frame = new MyFrame(wxT("CustomKeyboard"));
    SetTopWindow(frame);
    Database::Get()->GenerateGraphs();
    return true;
}

int MyApp::OnExit()
{
    Logger::CSingleton::Destroy();
    Settings::CSingleton::Destroy();
    CustomMacro::CSingleton::Destroy();
    Server::CSingleton::Destroy();
    Sensors::CSingleton::Destroy();
    Database::CSingleton::Destroy();
    StructParser::CSingleton::Destroy();
    PrintScreenSaver::CSingleton::Destroy();
    DirectoryBackup::CSingleton::Destroy();
    return true;
}

void MyApp::OnUnhandledException()
{
    try
    {
        throw;
    }
    catch(std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "std::exception caught", MB_OK);
    }
    catch(...)
    {
        MessageBoxA(NULL, "Unknown exception", "exception caught", MB_OK);
    }
}