#include "pch.hpp"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if(!wxApp::OnInit())
        return false;

    ExceptionHandler::Register();

    can_entry = std::make_unique<CanEntryHandler>(xml, rx_xml, mapping_xml);
    cmd_executor = std::make_unique<CmdExecutor>();
    did_handler = std::make_unique<DidHandler>(did_xml_loader, did_xml_chace_loader, can_entry.get());

    Settings::Get()->Init();
    SerialPort::Get()->Init();
    CanSerialPort::Get()->Init();
    ModbusMasterSerialPort::Get()->Init();
    Server::Get()->Init();
    Sensors::Get()->Init();
    StructParser::Get()->Init();
    PrintScreenSaver::Get()->Init();
    DirectoryBackup::Get()->Init();
    MacroRecorder::Get()->Init();
    SerialForwarder::Get()->Init();
    CorsairHid::Get()->Init();

    can_entry->Init();
    cmd_executor->Init();
    did_handler->Init();

    if(!wxTaskBarIcon::IsAvailable())
        LOG(LogLevel::Warning, "There appears to be no system tray support in your current environment. This app may not behave as expected.");
    MyFrame* frame = new MyFrame(wxT("CustomKeyboard"));
    SetTopWindow(frame);
    is_init_finished = true;
    TerminalHotkey::Get()->UpdateHotkeyRegistration();
    return true;
}

int MyApp::OnExit()
{
    is_init_finished = false;
    
    CanSerialPort::CSingleton::Destroy();

    can_entry.reset(nullptr);
    cmd_executor.reset(nullptr);
    did_handler.reset(nullptr);

    IdlePowerSaver::CSingleton::Destroy();  /* Restore CPU power to 100%, this has to be destructed before Logger */
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
    ModbusMasterSerialPort::CSingleton::Destroy();
    CorsairHid::CSingleton::Destroy();
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