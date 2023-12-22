#pragma once

#ifdef _WIN32
#include "resource.h"
#endif

#include <wx/wx.h>
#include "CanEntryHandler.hpp"
#include "CmdExecutor.hpp"
#include "DidHandler.hpp"
#include "ModbusHandler.hpp"
//#include "DataSender.hpp"

class MyApp : public wxApp
{
public:
    bool OnInit() override;
    int OnExit() override;
    void OnUnhandledException() override;

    // !\brief Helper variable for logger to avoid crash when inserting log messages befor logger frame is created
    bool is_init_finished = false;

    XmlCanEntryLoader xml;
    XmlCanRxEntryLoader rx_xml;
    XmlCanMappingLoader mapping_xml;
    XmlDidLoader did_xml_loader;
    XmlDidCacheLoader did_xml_chace_loader;
    XmlModbusEnteryLoader modbus_entry_loader;
    //XmlDataEntryLoader data_entry_loader;
    std::unique_ptr<CanEntryHandler> can_entry;
    std::unique_ptr<CmdExecutor> cmd_executor;
    std::unique_ptr<DidHandler> did_handler;
    std::unique_ptr<ModbusEntryHandler> modbus_handler;
    //std::unique_ptr<DataEntryHandler> data_entry;
};