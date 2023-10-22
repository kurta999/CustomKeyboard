#include "pch.hpp"

bool XmlModbusEnteryLoader::Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
    ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries)
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

            enum ModbusItemTypes { COIL, INPUT_STATUS, INPUT_REGISTER, HOLDING_REGISTER, UNKNOWN};
            ModbusItemTypes register_type = ModbusItemTypes::UNKNOWN;
            ModbusBitfieldType register_value_type = ModbusBitfieldType::MBT_BOOL;
            ModbusItemType* item = nullptr;
            int pos = 0;
            for(const boost::property_tree::ptree::value_type& m : v.second) /* loop over each nested child */
            {
                if(m.first == "Coil")
                {
                    register_type = COIL;
                    item = &coils;
                }
                else if(m.first == "InputStatus")
                {
                    register_type = INPUT_STATUS;
                    item = &input_status;
                }
                else if(m.first == "Input")
                {
                    register_type = INPUT_REGISTER;
                    item = &input;
                    register_value_type = ModbusBitfieldType::MBT_UI16;
                }
                else if(m.first == "Holding")
                {
                    register_type = HOLDING_REGISTER;
                    item = &holding;
                    register_value_type = ModbusBitfieldType::MBT_UI16;
                }

                if(register_type != UNKNOWN)
                {
                    std::string name = m.second.get_child("Name").get_value<std::string>();
                    bool last_val = m.second.get_child("LastVal").get_value<bool>();

                    boost::optional<std::string> color;
                    boost::optional<std::string> bg_color;
                    boost::optional<bool> is_bold;
                    boost::optional<float> is_scale;
                    boost::optional<std::string> is_font_face;
                    utils::xml::ReadChildIfexists<std::string>(m, "Color", color);
                    utils::xml::ReadChildIfexists<std::string>(m, "BackgroundColor", bg_color);
                    utils::xml::ReadChildIfexists<float>(m, "Scale", is_scale);
                    utils::xml::ReadChildIfexists<std::string>(m, "FontFace", is_font_face);

                    std::optional<uint32_t> color_;
                    std::optional<uint32_t> bg_color_;
                    std::optional<bool> is_bold_;
                    std::optional<float> is_scale_;
                    std::optional<std::string> is_font_face_;

                    if(color.has_value())
                        color_ = utils::ColorStringToInt(*color);
                    if(bg_color.has_value())
                        bg_color_ = utils::ColorStringToInt(*bg_color);
                    if(is_bold.has_value() && *is_bold)
                        is_bold_ = true;
                    if(is_scale.has_value())
                        is_scale_ = *is_scale;
                    if(is_font_face.has_value())
                        is_font_face_ = *is_font_face;

                    std::unique_ptr<ModbusItem> ptr = std::make_unique<ModbusItem>(name, register_value_type, last_val, color_, bg_color_, is_bold_, is_scale_, is_font_face_);
                    item->push_back(std::move(ptr));

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

bool XmlModbusEnteryLoader::Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
    ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("Modbus", boost::property_tree::ptree{});
    root_node.add("SlaveAddress", slave_id);
    root_node.add("NumCoils", num_entries.coils);
    root_node.add("NumInputStatus", num_entries.inputStatus);
    root_node.add("NumHoldingRegisters", num_entries.holdingRegisters);
    root_node.add("NumInputRegisters", num_entries.inputRegisters);
    root_node.add("SlaveAddress", slave_id);
    auto& coils_node = root_node.add_child("Coils", boost::property_tree::ptree{});

    ModbusItemType* items[] = { &coils, &input_status, &holding, &input };
    std::string child_names[] = { "Coils", "InputStatus", "Holding", "Input" };
    std::string subchild_names[] = { "Coils", "InputStatuses", "HoldingRegisters", "InputRegisters" };

    int id = 0;
    for(auto& i : items)
    {
        auto& register_node = root_node.add_child(child_names[id], boost::property_tree::ptree{});
        for(auto& m : coils)
        {
            auto& sub_node = register_node.add_child(subchild_names[id], boost::property_tree::ptree{});
            sub_node.add("Name", m->m_Name);
            sub_node.add("LastVal", m->m_Value);

            if(m->m_color)
                sub_node.add("Color", utils::ColorIntToString(*m->m_color));
            if(m->m_bg_color)
                sub_node.add("BackgroundColor", utils::ColorIntToString(*m->m_bg_color));
            if(m->m_is_bold)
                sub_node.add("Bold", "1");
            if(m->m_scale != 1.0f)
                sub_node.add("Scale", std::format("{:.1f}", m->m_scale));
            if(!m->m_font_face.empty())
                sub_node.add("FontFace", m->m_font_face);

        }
        id++;
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
    start_time = std::chrono::steady_clock::now();
    m_Serial = std::make_unique<ModbusMasterSerialPort>();
    m_workerModbus = std::make_unique<std::jthread>(std::bind_front(&ModbusEntryHandler::ModbusWorker, this));
    utils::SetThreadName(*m_workerModbus, "ModbusWorker");
}

ModbusEntryHandler::~ModbusEntryHandler()
{
    m_isMainThreadPaused = false;
    cv.notify_all();
    if(m_workerModbus)
        m_workerModbus.reset();
    m_Serial.reset();
}

void ModbusEntryHandler::Init()
{
    m_ModbusEntryLoader.Load(m_DefaultConfigName, m_slaveId, m_coils, m_inputStatus, m_Holding, m_Input, m_numEntries);

    is_recoding = auto_recording;  /* Toggle auto recording */
}

void ModbusEntryHandler::Save()
{
    m_ModbusEntryLoader.Save("Modbus2.xml", m_slaveId, m_coils, m_inputStatus, m_Holding, m_Input, m_numEntries);
}

void ModbusEntryHandler::SetModbusHelper(IModbusHelper* helper)
{
    m_helper = helper;
    m_Serial->SetHelper(helper);
}

void ModbusEntryHandler::SetEnabled(bool enable)
{
    m_Serial->SetEnabled(enable);
    is_enabled = enable;
    if(enable)
        m_Serial->Init();
}

bool ModbusEntryHandler::IsEnabled() const
{
    return is_enabled;
}

void ModbusEntryHandler::SetPollingStatus(bool is_active)
{
    std::lock_guard lock(m);
    m_isMainThreadPaused = !is_active;
    if(is_active)
        cv.notify_one();
}

void ModbusEntryHandler::ToggleAutoSend(bool toggle)
{
    auto_send = toggle;
    SetPollingStatus(toggle);
}

void ModbusEntryHandler::ToggleRecording(bool toggle, bool is_pause)
{
    std::scoped_lock lock{ m };
    is_recoding = toggle;

    if(!is_pause && !toggle)
    {
        tx_frame_cnt = rx_frame_cnt = 0;
        m_LogEntries.clear();
    }
}

void ModbusEntryHandler::ClearRecording()
{
    std::scoped_lock lock{ m };
    tx_frame_cnt = rx_frame_cnt = 0;
    m_LogEntries.clear();
}

bool ModbusEntryHandler::SaveRecordingToFile(std::filesystem::path& path)
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::scoped_lock lock{ m };
    bool ret = false;
    if(!m_LogEntries.empty())
    {
        std::ofstream out(path, std::ofstream::binary);
        if(out.is_open())
        {
            out << "Time,Direction,FunctionCode,DataSize,Data\n";
            for(auto& i : m_LogEntries)
            {
                std::string hex;
                utils::ConvertHexBufferToString(i->data, hex);
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(i->last_execution - start_time).count();
                out << std::format("{:.3f},{},{},{},{}\n", static_cast<double>(elapsed) / 1000.0, i->direction == 0 ? "TX" : "RX",
                    static_cast<uint32_t>(i->fcode), i->data.size(), hex);
            }
            out.flush();
            ret = true;
        }
        else
        {
            LOG(LogLevel::Error, "Failed to open file for saving Modbus recording: {}", path.generic_string());
        }
    }

    if(ret)
    {
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        std::unique_lock lock(frame->mtx);
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::ModbusLogSaved), dif, path.generic_string() });
    }
    return ret;
}

void ModbusEntryHandler::HandleBoolReading(std::vector<uint8_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel)
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

            if(coil_pos_in_vec >= num_items)
            {
                to_exit = true;
                break;
            }

            if(coil_pos_in_vec < items.size())
            {
                if(items[coil_pos_in_vec]->m_Value != bit_val)
                {
                    items[coil_pos_in_vec]->m_Value = bit_val;
                    changed_rows.push_back(coil_pos_in_vec);
                }
            }
            else
            {
                std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>("Unknown", MBT_BOOL, bit_val);
                items.push_back(std::move(item));
                changed_rows.push_back(items.size() - 1);
            }
        }

        if(to_exit)
            break;
        cnt++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);
}

void ModbusEntryHandler::HandleRegisterReading(std::vector<uint16_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel)
{
    std::vector<uint8_t> changed_rows;
    int coil_pos_in_vec = 0;
    for(auto& i : reg)
    {
        if(coil_pos_in_vec >= num_items)
        {
            break;
        }

        if(coil_pos_in_vec < items.size())
        {
            if(items[coil_pos_in_vec]->m_Value != i)
            {
                items[coil_pos_in_vec]->m_Value = i;
                changed_rows.push_back(coil_pos_in_vec);
            }
        }
        else
        {
            std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>("Unknown", MBT_UI16, i);
            items.push_back(std::move(item));

            changed_rows.push_back(items.size() - 1);
        }
        coil_pos_in_vec++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);
}

void ModbusEntryHandler::WaitIfPaused()
{
    std::unique_lock lk(m);
    cv.wait(lk, [this] { return !m_isMainThreadPaused; });
}

void ModbusEntryHandler::HandlePolling()
{
    ModbusItemPanel* panel_coil = nullptr;
    ModbusItemPanel* panel_input = nullptr;
    ModbusItemPanel* panel_holding = nullptr;
    ModbusItemPanel* panel_inputReg = nullptr;
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(frame && frame->is_initialized && frame->modbus_master_panel)
    {
        panel_coil = frame->modbus_master_panel->data_panel->m_coil;
        panel_input = frame->modbus_master_panel->data_panel->m_input;
        panel_holding = frame->modbus_master_panel->data_panel->m_holding;
        panel_inputReg = frame->modbus_master_panel->data_panel->m_inputReg;
    }

    {
        std::vector<uint8_t> reg = m_Serial->ReadCoilStatus(m_slaveId, 0, m_numEntries.coils);
        if(!reg.empty())
            HandleBoolReading(reg, m_coils, m_numEntries.coils, panel_coil);
        else
            err_frame_cnt++;
    }

    {
        std::vector<uint8_t> reg = m_Serial->ReadInputStatus(m_slaveId, 0, m_numEntries.inputStatus);
        if(!reg.empty())
            HandleBoolReading(reg, m_inputStatus, m_numEntries.inputStatus, panel_input);
        else
            err_frame_cnt++;
    }

    {
        std::vector<uint16_t> reg = m_Serial->ReadHoldingRegister(m_slaveId, 0, m_numEntries.holdingRegisters);
        if(!reg.empty())
            HandleRegisterReading(reg, m_Holding, m_numEntries.holdingRegisters, panel_holding);
        else
            err_frame_cnt++;
    }

    {
        std::vector<uint16_t> reg = m_Serial->ReadInputRegister(m_slaveId, 0, m_numEntries.inputRegisters);
        if(!reg.empty())
            HandleRegisterReading(reg, m_Input, m_numEntries.inputRegisters, panel_inputReg);
        else
            err_frame_cnt++;
    }

    tx_frame_cnt += 4;
    rx_frame_cnt += 4;
}

void ModbusEntryHandler::HandleWrites()
{
    for(auto& c : m_pendingCoilWrites)
    {
        std::vector<uint8_t> reg = m_Serial->ForceSingleCoil(m_slaveId, c.first, c.second);
        if(!reg.empty())
        {
            tx_frame_cnt++;
            rx_frame_cnt++;
        }
        else
        {
            err_frame_cnt++;
        }
    }
    for(auto& c : m_pendingHoldingWrites)
    {
        std::vector<uint16_t> vec;
        vec.push_back(c.second & 0xFFFF);
        std::vector<uint8_t> reg = m_Serial->WriteHoldingRegister(m_slaveId, c.first, 1, vec);
        if(!reg.empty())
        {
            tx_frame_cnt++;
            rx_frame_cnt++;
        }
        else
        {
            err_frame_cnt++;
        }
    }
    m_pendingCoilWrites.clear();
    m_pendingHoldingWrites.clear();
}

void ModbusEntryHandler::ModbusWorker(std::stop_token token)
{
    m_Serial->SetStopToken(token);
    std::this_thread::sleep_for(200ms);
    while(!token.stop_requested())
    {
        WaitIfPaused();

        HandlePolling();
        //HandleWrites();
            
        {
            std::unique_lock lock(m);
            auto now = std::chrono::system_clock::now();
            cv.wait_until(lock, token, now + std::chrono::milliseconds(m_pollingRate), []() { return 0 == 1; });
        }
    }
}