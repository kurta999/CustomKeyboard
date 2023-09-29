#include "pch.hpp"

bool XmlModbusEnteryLoader::Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
    ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml("Modbus.xml", pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("Modbus")) /* loop over each Frame */
        {
            if(v.first == "NumCoils")
            {
                num_entries.coils = v.second.get_value<size_t>(0);
                continue;
            }
            if(v.first == "NumInputStatus")
            {
                num_entries.inputStatus = v.second.get_value<size_t>(0);
                continue;
            }
            if(v.first == "NumHoldingRegisters")
            {
                num_entries.holdingRegisters = v.second.get_value<size_t>(0);
                continue;
            }
            if(v.first == "NumInputRegisters")
            {
                num_entries.inputRegisters = v.second.get_value<size_t>(0);
                continue;
            }
            if(v.first == "SlaveAddress")
            {
                slave_id = v.second.get_value<size_t>(0);
                continue;
            }

            int pos = 0;
            for(const boost::property_tree::ptree::value_type& m : v.second) /* loop over each nested child */
            {
                if(m.first == "Coil")
                {
                    std::string name = m.second.get_child("Name").get_value<std::string>();
                    bool last_val = m.second.get_child("LastVal").get_value<bool>();

                    coils.push_back({ name, last_val });
                }
                else if(m.first == "InputStatus")
                {
                    std::string name = m.second.get_child("Name").get_value<std::string>();
                    bool last_val = m.second.get_child("LastVal").get_value<bool>();

                    input_status.push_back({ name, last_val });
                }
                else if(m.first == "Holding")
                {
                    std::string name = m.second.get_child("Name").get_value<std::string>();
                    uint64_t last_val = m.second.get_child("LastVal").get_value<uint64_t>();

                    std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>(name, last_val, 0, 0, false, 0.0f);
                    holding.push_back(std::move(item));
                }
            }
        }
    }
    catch(const boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlModbusEnteryLoader::Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
    ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("Modbus", boost::property_tree::ptree{});
    root_node.add("SlaveAddress", slave_id);
    root_node.add("NumCoils", num_entries.coils);
    root_node.add("NumInputStatus", num_entries.inputStatus);
    root_node.add("NumHoldingRegisters", num_entries.holdingRegisters);
    root_node.add("NumInputRegisters", num_entries.inputRegisters);
    auto& coils_node = root_node.add_child("Coils", boost::property_tree::ptree{});
    for(auto& m : coils)
    {
        auto& coil_node = coils_node.add_child("Coil", boost::property_tree::ptree{});
        coil_node.add("Name", m.first);
        coil_node.add("LastVal", m.second);
    }
    auto& inputstatuses_node = root_node.add_child("InputStatuses", boost::property_tree::ptree{});
    for(auto& m : input_status)
    {
        auto& input_node = inputstatuses_node.add_child("InputStatus", boost::property_tree::ptree{});
        input_node.add("Name", m.first);
        input_node.add("LastVal", m.second);
    }

    try
    {
        boost::property_tree::write_xml(path.generic_string(), pt, std::locale(),
            boost::property_tree::xml_writer_make_settings<boost::property_tree::ptree::key_type>('\t', 1));
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

ModbusEntryHandler::ModbusEntryHandler(XmlModbusEnteryLoader& loader) : m_ModbusEntryLoader(loader)
{
    if(ModbusMasterSerialPort::Get()->IsEnabled())
        m_workerModbus = std::make_unique<std::jthread>(std::bind_front(&ModbusEntryHandler::ModbusWorker, this));
}

ModbusEntryHandler::~ModbusEntryHandler()
{
    if(m_workerModbus)
        m_workerModbus->request_stop();
}

void ModbusEntryHandler::Init()
{
    m_ModbusEntryLoader.Load(m_DefaultConfigName, m_slaveId, m_coils, m_inputStatus, m_Holding, m_Input, m_numEntries);
}

void ModbusEntryHandler::Save()
{
    m_ModbusEntryLoader.Save("Modbus2.xml", m_slaveId, m_coils, m_inputStatus, m_Holding, m_Input, m_numEntries);
}

void ModbusEntryHandler::HandleBoolReading(std::vector<uint8_t>& reg, ModbusCoils& coils, size_t num_coils, CoilStatusPanel* panel)
{
    std::vector<uint8_t> changed_rows;
    int cnt = 0;
    bool to_exit = false;
    for(auto& i : reg)
    {
        for(uint8_t x = 0; x != 8; x++)
        {
            bool bit_val = reg[cnt] & (1 << x);
            size_t coil_pos_in_vec = (cnt * 8) + x;

            if(coil_pos_in_vec >= num_coils)
            {
                to_exit = true;
                break;
            }

            if(coil_pos_in_vec < coils.size())
            {
                if(coils[coil_pos_in_vec].second != bit_val)
                {
                    coils[coil_pos_in_vec].second = bit_val;
                    changed_rows.push_back(coil_pos_in_vec);
                }
            }
            else
            {
                coils.push_back({ "Unknown", bit_val });
                changed_rows.push_back(coils.size() - 1);
            }
        }

        if(to_exit)
            break;
        cnt++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);
}

void ModbusEntryHandler::HandleRegisterReading(std::vector<uint16_t>& reg, ModbusHoldingRegisters& holding, size_t num_coils, ModbusRegisterPanel* panel)
{
    std::vector<uint8_t> changed_rows;
    int coil_pos_in_vec = 0;
    bool to_exit = false;
    for(auto& i : reg)
    {
        if(coil_pos_in_vec >= num_coils)
        {
            to_exit = true;
            break;
        }

        if(coil_pos_in_vec < holding.size())
        {
            if(holding[coil_pos_in_vec]->m_Value != i)
            {
                holding[coil_pos_in_vec]->m_Value = i;
                changed_rows.push_back(coil_pos_in_vec);
            }
        }
        else
        {
            std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>("Unknown", i, 0, 0, false, 0.0f);
            holding.push_back(std::move(item));

            changed_rows.push_back(holding.size() - 1);
        }
        coil_pos_in_vec++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);
}

void ModbusEntryHandler::ModbusWorker(std::stop_token token)
{
    std::this_thread::sleep_for(200ms);
    while(!token.stop_requested())
    {
        CoilStatusPanel* panel_coil = nullptr;
        CoilStatusPanel* panel_input = nullptr;
        ModbusRegisterPanel* panel_holding = nullptr;
        ModbusRegisterPanel* panel_inputReg = nullptr;
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame && frame->is_initialized && frame->modbus_master_panel)
        {
            panel_coil = frame->modbus_master_panel->m_coil;
            panel_input = frame->modbus_master_panel->m_input;
            panel_holding = frame->modbus_master_panel->m_holding;
            panel_inputReg = frame->modbus_master_panel->m_inputReg;
        }


        {
            std::vector<uint8_t> reg = ModbusMasterSerialPort::Get()->ReadCoilStatus(m_slaveId, 0, m_numEntries.coils);
            HandleBoolReading(reg, m_coils, m_numEntries.coils, panel_coil);
        }

        {
            std::vector<uint8_t> reg = ModbusMasterSerialPort::Get()->ReadInputStatus(m_slaveId, 0, m_numEntries.inputStatus);
            HandleBoolReading(reg, m_inputStatus, m_numEntries.inputStatus, panel_input);
        }

        {
            std::vector<uint16_t> reg = ModbusMasterSerialPort::Get()->ReadHoldingRegister(m_slaveId, 0, m_numEntries.holdingRegisters);
            HandleRegisterReading(reg, m_Holding, m_numEntries.holdingRegisters, panel_holding);
        }

        {
            std::vector<uint16_t> reg = ModbusMasterSerialPort::Get()->ReadInputRegister(m_slaveId, 0, m_numEntries.inputRegisters);
            HandleRegisterReading(reg, m_Input, m_numEntries.inputRegisters, panel_holding);
        }

        for(auto& c : m_pendingCoilWrites)
        {
            ModbusMasterSerialPort::Get()->ForceSingleCoil(m_slaveId, c.first, c.second);
        }
        for(auto& c : m_pendingHoldingWrites)
        {
            std::vector<uint16_t> vec;
            vec.push_back(c.second & 0xFFFF);
            ModbusMasterSerialPort::Get()->WriteHoldingRegister(m_slaveId, c.first, 1, vec);
        }
        m_pendingCoilWrites.clear();
        m_pendingHoldingWrites.clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(m_pollingRate));
    }
}