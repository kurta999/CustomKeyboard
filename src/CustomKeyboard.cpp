#include "pch.hpp"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if(!wxApp::OnInit())
        return false;

    Settings::Get()->Init();
    SerialPort::Get()->Init();
    Server::Get()->Init();
    Sensors::Get()->Init();
    StructParser::Get()->Init();
    PrintScreenSaver::Get()->Init();
    DirectoryBackup::Get()->Init();
    MacroRecorder::Get()->Init();
    SerialForwarder::Get()->Init();
    CryptoPrice::Get()->Init();
    if(!wxTaskBarIcon::IsAvailable())
        LOGMSG(normal, "There appears to be no system tray support in your current environment. This app may not behave as expected.");
    MyFrame* frame = new MyFrame(wxT("CustomKeyboard"));
    SetTopWindow(frame);
    is_init_finished = true;
    return true;
}

int MyApp::OnExit()
{
    is_init_finished = false;
    Settings::CSingleton::Destroy();
    CustomMacro::CSingleton::Destroy();
    Server::CSingleton::Destroy();
    Sensors::CSingleton::Destroy();
    DatabaseLogic::CSingleton::Destroy();
    StructParser::CSingleton::Destroy();
    PrintScreenSaver::CSingleton::Destroy();
    DirectoryBackup::CSingleton::Destroy();
    MacroRecorder::CSingleton::Destroy();
    DatabaseLogic::CSingleton::Destroy();
    SerialForwarder::CSingleton::Destroy();
    SerialPort::CSingleton::Destroy();
    Logger::CSingleton::Destroy();
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
#ifdef _WIN32
        MessageBoxA(NULL, e.what(), "std::exception caught", MB_OK);
#endif
    }
    catch(...)
    {
#ifdef _WIN32
        MessageBoxA(NULL, "Unknown exception", "exception caught", MB_OK);
#endif
    }
}