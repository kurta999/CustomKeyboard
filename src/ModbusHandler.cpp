#include "pch.hpp"

constexpr int NUM_EVENTLOG_ENTRIES = 8;
constexpr int EVENTLOG_ENTRY_SIZE = 9;
constexpr int EVENTLOG_BUFFER_SIZE = NUM_EVENTLOG_ENTRIES * EVENTLOG_ENTRY_SIZE;

bool XmlModbusEnteryLoader::Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
    ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        size_t register_offset_coils = 0;
        size_t register_offset_input_status = 0;
        size_t register_offset_holding = 0;
        size_t register_offset_input = 0;

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
            if(v.first == "NumInput")
            {
                num_entries.Input = v.second.get_value<size_t>(0);
                continue;
            }            
            if(v.first == "InputStatusOffset")
            {
                num_entries.inputStatusOffset = v.second.get_value<uint16_t>(0);
                continue;
            }
            if(v.first == "InputOffset")
            {
                num_entries.inputOffset = v.second.get_value<uint16_t>(0);
                continue;
            }
            if(v.first == "HoldingOffset")
            {
                num_entries.holdingOffset = v.second.get_value<uint16_t>(0);
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
            size_t* offset = nullptr;
            int pos = 0;

            for(const boost::property_tree::ptree::value_type& m : v.second) /* loop over each nested child */
            {
                if(m.first == "Coil")
                {
                    register_type = COIL;
                    item = &coils;
                    offset = &register_offset_coils;
                }
                else if(m.first == "InputStatus")
                {
                    register_type = INPUT_STATUS;
                    item = &input_status;
                    offset = &register_offset_input_status;
                }
                else if(m.first == "Input")
                {
                    register_type = INPUT_REGISTER;
                    item = &input;
                    offset = &register_offset_input;
                    register_value_type = ModbusBitfieldType::MBT_UI16;
                }
                else if(m.first == "Holding")
                {
                    register_type = HOLDING_REGISTER;
                    item = &holding;
                    offset = &register_offset_holding;
                    register_value_type = ModbusBitfieldType::MBT_UI16;
                }
                else
                {
                    LOG(LogLevel::Warning, "Invalid modbus register child in Modbus.xml: {}", m.first);
                    continue;
                }

                if(register_type != UNKNOWN)
                {
                    std::string name = m.second.get_child("Name").get_value<std::string>();
                    boost::optional<uint8_t> fav_child_val = m.second.get_optional<uint8_t>("FavLevel");
                    uint8_t fav_level = 0;
                    if (fav_child_val)
                        fav_level = *fav_child_val;

                    bool last_val = m.second.get_child("LastVal").get_value<bool>();
                    boost::optional<std::string> data_type = m.second.get_optional<std::string>("DataType");
                    if (data_type)
                    {
                        register_value_type = GetTypeFromString(*data_type);
                    }

                    ModbusValueFormat val_format = ModbusValueFormat::MVF_DEC;
                    boost::optional<std::string> val_format_str = m.second.get_optional<std::string>("Format");
                    if (val_format_str)
                    {
                        if (*val_format_str == "dec")
                        {
                            val_format = ModbusValueFormat::MVF_DEC;
                        }
                        else if (*val_format_str == "hex")
                        {
                            val_format = ModbusValueFormat::MVF_HEX;
                        }
                        else if (*val_format_str == "bin")
                        {
                            val_format = ModbusValueFormat::MVF_BIN;
                        }
                    }

                    boost::optional<boost::multiprecision::int128_t> min_val = m.second.get_optional<boost::multiprecision::int128_t>("Min");

                    boost::optional<int64_t> min_val_child = m.second.get_optional<int64_t>("Min");
                    boost::optional<int64_t> max_val_child = m.second.get_optional<int64_t>("Max");
                    std::string description;
                    boost::optional<std::string> description_child = m.second.get_optional<std::string>("Desc");
                    if (description_child)
                    {
                        description = *description_child;
                        boost::algorithm::replace_all(description, "\\n", "\n");  /* Fix for newlines */
                    }

                    boost::optional<std::string> color;
                    boost::optional<std::string> bg_color;
                    boost::optional<bool> is_bold;
                    boost::optional<float> is_scale;
                    boost::optional<std::string> is_font_face;
                    utils::xml::ReadChildIfexists<std::string>(m, "Color", color);
                    utils::xml::ReadChildIfexists<std::string>(m, "BackgroundColor", bg_color);
                    utils::xml::ReadChildIfexists<float>(m, "Scale", is_scale);
                    utils::xml::ReadChildIfexists<bool>(m, "Bold", is_bold);
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

                    ModbusMapping mapping;
                    size_t calculated_size = 0;
                    for (const boost::property_tree::ptree::value_type& x : m.second) /* loop over each nested child */
                    {
                        if (x.first == "Mapping")
                        {
                            uint8_t offset = x.second.get<uint8_t>("<xmlattr>.offset");
                            uint8_t len = x.second.get<uint8_t>("<xmlattr>.len");
                            std::string type = x.second.get<std::string>("<xmlattr>.type");
                            std::string name = x.second.get_value<std::string>();
                            int64_t min_val = std::numeric_limits<int64_t>::min();
                            int64_t max_val = std::numeric_limits<int64_t>::max();
                            uint32_t color = wxBLACK->GetRGB();
                            uint32_t bg_color = DEFAULT_TXTCTRL_BACKGROUND;
                            bool is_bold = false;
                            float scale = 1.0f;
                            
                            ModbusBitfieldType bitfield_type = GetTypeFromString(type);
                            if (bitfield_type == MBT_INVALID)
                            {
                                LOG(LogLevel::Warning, "Invalid type used for frame mapping. Type: {}", type);
                                continue;
                            }

                            boost::optional<int64_t> min_val_child = x.second.get_optional<int64_t>("<xmlattr>.min");
                            boost::optional<int64_t> max_val_child = x.second.get_optional<int64_t>("<xmlattr>.max");
                            boost::optional<std::string> color_child = x.second.get_optional<std::string>("<xmlattr>.color");
                            boost::optional<std::string> bg_color_child = x.second.get_optional<std::string>("<xmlattr>.bg_color");
                            boost::optional<bool> is_bold_child = x.second.get_optional<bool>("<xmlattr>.bold");
                            boost::optional<float> scale_child = x.second.get_optional<float>("<xmlattr>.scale");
                            /*
                            if (min_val_child)
                            {
                                min_val = *min_val_child;
                                if (min_val < GetMinMaxForType(bitfield_type).first)
                                    min_val = GetMinMaxForType(bitfield_type).first;
                            }
                            else
                                min_val = GetMinMaxForType(bitfield_type).first;

                            if (max_val_child)
                            {
                                max_val = *max_val_child;
                                if (max_val > GetMinMaxForType(bitfield_type).second)
                                    max_val = GetMinMaxForType(bitfield_type).second;
                            }
                            else
                                max_val = GetMinMaxForType(bitfield_type).second;
                                */
                            if (color_child)
                                color = utils::ColorStringToInt(*color_child);
                            if (bg_color_child)
                                bg_color = utils::ColorStringToInt(*bg_color_child);
                            if (is_bold_child)
                                is_bold = *is_bold_child;
                            if (scale_child)
                                scale = *scale_child;

                            std::string description;
                            boost::optional<std::string> description_child = m.second.get_optional<std::string>("<xmlattr>.desc");
                            if (description_child)
                            {
                                description = *description_child;
                                boost::algorithm::replace_all(description, "\\n", "\n");  /* Fix for newlines */
                            }

                            mapping.try_emplace(offset, std::make_unique<ModbusMap>(std::move(name), bitfield_type, len, min_val, max_val, std::move(description),
                                color, bg_color, is_bold, scale));
                            calculated_size += len;

                        }
                    }

                    std::unique_ptr<ModbusItem> ptr = std::make_unique<ModbusItem>(name, fav_level, *offset, register_value_type, val_format, description, mapping, 0, 0, last_val,
                        color_, bg_color_, is_bold_, is_scale_, is_font_face_);
                    *offset += ptr->GetSize();
                    item->push_back(std::move(ptr));
                }
            }
        }
        
        if (num_entries.coils == 0xFFFF)
            num_entries.coils = register_offset_coils;
        if (num_entries.inputStatus == 0xFFFF)
            num_entries.inputStatus = register_offset_input_status;
        if (num_entries.holdingRegisters == 0xFFFF)
            num_entries.holdingRegisters = register_offset_holding;
        if (num_entries.Input == 0xFFFF)
            num_entries.Input = register_offset_input;
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
    root_node.add("NumInput", num_entries.Input);
    root_node.add("SlaveAddress", slave_id);

    ModbusItemType* items[] = { &coils, &input_status, &holding, &input };
    std::string child_names[] = { "Coils", "InputStatus", "Holding", "Input" };
    std::string subchild_names[] = { "Coils", "InputStatuses", "HoldingRegisters", "Input" };

    int id = 0;
    for(auto& i : items)
    {
        auto& register_node = root_node.add_child(child_names[id], boost::property_tree::ptree{});
        for(auto& m : *i)
        {
            auto& sub_node = register_node.add_child(subchild_names[id], boost::property_tree::ptree{});
            sub_node.add("Name", m->m_Name);
            sub_node.add("FavLevel", m->m_FavLevel);
            
            std::string_view type = GetStringFromType(m->m_Type);
            sub_node.add("DataType", type);

            std::string value_type_str = "dec";
            switch (m->m_Format)
            {
                case ModbusValueFormat::MVF_DEC:
                {
                    value_type_str = "dec";
                    break;
                }
                case ModbusValueFormat::MVF_HEX:
                {
                    value_type_str = "hex";
                    break;
                }
                case ModbusValueFormat::MVF_BIN:
                {
                    value_type_str = "bin";
                    break;
                }
            }
            sub_node.add("Format", value_type_str);

            sub_node.add("Min", m->m_Min);
            sub_node.add("Max", m->m_Max);
            if (!m->m_Desc.empty())
            {
                std::string desc_str = m->m_Desc;
                boost::algorithm::replace_all(desc_str, "\n", "\\n");  /* Fix for newlines */
                sub_node.add("Desc", desc_str);
            }

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

ModbusBitfieldType XmlModbusEnteryLoader::GetTypeFromString(const std::string_view& input)
{
    auto ret = std::find_if(m_ModbusBitfieldTypeMap.cbegin(), m_ModbusBitfieldTypeMap.cend(), [&input](const auto& item) { return item.second == input; });
    if (ret == m_ModbusBitfieldTypeMap.cend())
        return ModbusBitfieldType::MBT_INVALID;
    return ret->first;
}

const std::string_view XmlModbusEnteryLoader::GetStringFromType(ModbusBitfieldType type)
{
    auto it = m_ModbusBitfieldTypeMap.find(type);
    if (it != m_ModbusBitfieldTypeMap.end())
        return it->second;
    return m_ModbusBitfieldTypeMap[MBT_INVALID];
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
        tx_frame_cnt = rx_frame_cnt = err_frame_cnt = 0;
        m_LogEntries.clear();
    }
}

void ModbusEntryHandler::ClearRecording()
{
    std::scoped_lock lock{ m };
    tx_frame_cnt = rx_frame_cnt = err_frame_cnt = 0;
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

bool ModbusEntryHandler::SaveSpecialRecordingToFile(std::filesystem::path& path)
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::scoped_lock lock{ m };
    bool ret = false;
    if(!m_EventLogEntries.empty())
    {
        std::ofstream out(path, std::ofstream::binary);
        if(out.is_open())
        {
            out << "Time,DataSize,Data\n";
            for(auto& i : m_EventLogEntries)
            {
                std::string hex;
                utils::ConvertHexBufferToString(i->data, hex);
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(i->last_execution - start_time).count();
                out << std::format("{:.3f},{}\n", static_cast<double>(elapsed) / 1000.0, hex);
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

template <typename T> void ModbusEntryHandler::HandleBitReading(size_t id, bool is_holding, std::unique_ptr<ModbusMap>& m, size_t offset, ModbusBitfieldInfo& info)
{
    uint8_t* register_pointer = is_holding ? (uint8_t*)&m_Holding[id]->m_Value : (uint8_t*)&m_Input[id]->m_Value;
    uint64_t value = get_bitfield(register_pointer, 8, offset, m->m_Size);
    T extracted_data = static_cast<T>(value);
    info.push_back({std::format("{}         (offset: {}, size: {}, range: {} - {})", m->m_Name, offset, m->m_Size, m->m_MinVal, m->m_MaxVal), std::to_string(extracted_data), m.get() });
}

template <typename T> void ModbusEntryHandler::HandleBitWriting(size_t id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::vector<std::string>& new_data)
{
    T raw_data;
    try
    {
        raw_data = static_cast<T>(std::stoi(new_data[pos]));
        set_bitfield(raw_data, offset, size, byte_array, sizeof(byte_array));
    }
    catch (const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid input for pos {}. Exception: {}", pos, e.what());
    }

    //DBG("%s - %d, %x ......... %d\n", m.second->m_Name.c_str(), m.first, raw_data, m.second->m_Size);
    pos++;
}

ModbusBitfieldInfo ModbusEntryHandler::GetMapForHolding(size_t id, bool is_holding)
{
    ModbusBitfieldInfo info;
    bool is_valid_id = is_holding ? m_Holding.size() >= id : m_Input.size() > id;
    if (is_valid_id)
    {
        for (auto& [offset, m] : m_Holding.at(id)->m_Mapping)
        {
            DBG("%d, %s\n", offset, m->m_Name.c_str());
            switch (m->m_Type)
            {
            case CBT_BOOL:
            case CBT_UI8:
            {
                HandleBitReading<uint8_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_I8:
            {
                HandleBitReading<int8_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_UI16:
            {
                HandleBitReading<uint16_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_I16:
            {
                HandleBitReading<int16_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_UI32:
            {
                HandleBitReading<uint32_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_I32:
            {
                HandleBitReading<int32_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_UI64:
            {
                HandleBitReading<uint64_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_I64:
            {
                HandleBitReading<int64_t>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_FLOAT:
            {
                HandleBitReading<float>(id, is_holding, m, offset, info);
                break;
            }
            case CBT_DOUBLE:
            {
                HandleBitReading<double>(id, is_holding, m, offset, info);
                break;
            }
            default:
            {
                break;
            }
            }
        }
    }
    return info;
}

void ModbusEntryHandler::ApplyEditingOnHolding(size_t id, std::vector<std::string> new_data)
{
    uint8_t cnt = 0;
    uint8_t byte_array[8] = {};
    memcpy(byte_array, &m_Holding.at(id)->m_Value, 8);

    if (m_Holding.size() >= id)
    {
        //LOG(LogLevel::Normal, "MapSize: {}, NewDataSize: {}", m_mapping[frame_id].size(), new_data.size());
        for (auto& [offset, m] : m_Holding.at(id)->m_Mapping)
        {
            switch (m->m_Type)
            {
            case CBT_BOOL:
            case CBT_UI8:
            {
                HandleBitWriting<uint8_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_I8:
            {
                HandleBitWriting<int8_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_UI16:
            {
                HandleBitWriting<uint16_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_I16:
            {
                HandleBitWriting<int16_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_UI32:
            {
                HandleBitWriting<uint32_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_I32:
            {
                HandleBitWriting<int32_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_UI64:
            {
                HandleBitWriting<uint64_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            case CBT_I64:
            {
                HandleBitWriting<int64_t>(id, cnt, offset, m->m_Size, byte_array, new_data);
                break;
            }
            }
        }
    }
    DBG("ok");

    uint64_t value = m_Holding.at(id)->m_Value = *(uint64_t*)&byte_array;
    EditHolding(id, value);
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
                if(items[coil_pos_in_vec]->m_Value != (uint64_t)bit_val)
                {
                    items[coil_pos_in_vec]->m_Value = bit_val;
                    changed_rows.push_back(coil_pos_in_vec);
                }
            }
            else
            {
                /*
                std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>("Unknown", 0, MBT_BOOL, bit_val);
                items.push_back(std::move(item));
                changed_rows.push_back(items.size() - 1);
                */
            }
        }

        if(to_exit)
            break;
        cnt++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);

    rx_frame_cnt++;
}

typedef union
{
    float asFloat;
    uint16_t asShorts[2];
} FloatUnion;

void ModbusEntryHandler::HandleRegisterReading(std::vector<uint16_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel)
{
    std::vector<uint8_t> changed_rows;
    int coil_pos_in_vec = 0;
    for(auto i = reg.begin(); i != reg.end(); i++)
    {
        if(coil_pos_in_vec >= num_items)
        {
            break;
        }

        if(coil_pos_in_vec < items.size())
        {
            if (items[coil_pos_in_vec]->m_Type == ModbusBitfieldType::MBT_UI16)
            {
                if (items[coil_pos_in_vec]->m_Value != *i)
                {
                    items[coil_pos_in_vec]->m_Value = *i;
                    changed_rows.push_back(coil_pos_in_vec);
                }
            }
            else if (items[coil_pos_in_vec]->m_Type == ModbusBitfieldType::MBT_UI32)
            {
                auto it = i + 1;
                if (it == reg.end())
                {
                    LOG(LogLevel::Warning, "End reached!");
                    break;
                }

                items[coil_pos_in_vec]->m_Value = (*(i + 1) << 16 | *i) & 0xFFFFFFFF;
                changed_rows.push_back(coil_pos_in_vec);
                //changed_rows.push_back(0xFF);

                i++;
            }
            else if (items[coil_pos_in_vec]->m_Type == ModbusBitfieldType::MBT_FLOAT)
            {
                auto it = i + 1;
                if (it == reg.end())
                {
                    LOG(LogLevel::Warning, "End reached!");
                    break;
                }

                FloatUnion un;
                un.asShorts[0] = *i;
                un.asShorts[1] = *(i + 1);
                items[coil_pos_in_vec]->m_fValue = un.asFloat;
                changed_rows.push_back(coil_pos_in_vec);
                //changed_rows.push_back(0xFF);

                i++;
            }
        }
        else
        {
            /*
            std::unique_ptr<ModbusItem> item = std::make_unique<ModbusItem>("Unknown", 0, MBT_UI16, *i);
            items.push_back(std::move(item));

            changed_rows.push_back(items.size() - 1);
            */
        }
        coil_pos_in_vec++;
    }

    if(panel)
        panel->UpdateChangesOnly(changed_rows);

    rx_frame_cnt++;
}

void ModbusEntryHandler::WaitUntilInited(std::stop_token token)
{
    std::unique_lock lk(m);
    cv.wait(lk, token, [this] { return m_Serial != nullptr && m_Serial->IsInstanceInited(); });
    std::this_thread::sleep_for(100ms);
}

void ModbusEntryHandler::WaitIfPaused(std::stop_token token)
{
    std::unique_lock lk(m);
    cv.wait(lk, token, [this] { return !m_isMainThreadPaused; });
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

    if (m_numEntries.coils > 0)
    {
        std::expected<std::vector<uint8_t>, ModbusError> reg = m_Serial->ReadCoilStatus(m_slaveId, 0, m_numEntries.coils);
        if (reg.has_value() && !reg->empty())
            HandleBoolReading(*reg, m_coils, m_numEntries.coils, panel_coil);
        else
            err_frame_cnt++;

        tx_frame_cnt++;
    }

    if(m_numEntries.inputStatus > 0)
    {
        std::expected<std::vector<uint8_t>, ModbusError> reg = m_Serial->ReadInputStatus(m_slaveId, m_numEntries.inputStatusOffset, m_numEntries.inputStatus);
        if (reg.has_value() && !reg->empty())
            HandleBoolReading(*reg, m_inputStatus, m_numEntries.inputStatus, panel_input);
        else
            err_frame_cnt++;

        tx_frame_cnt++;
    }

    if (m_numEntries.holdingRegisters > 0)
    {
        std::expected<std::vector<uint16_t>, ModbusError> reg = m_Serial->ReadHoldingRegisters(m_slaveId, m_numEntries.holdingOffset, m_numEntries.holdingRegisters);
        if (reg.has_value() && !reg->empty())
            HandleRegisterReading(*reg, m_Holding, m_numEntries.holdingRegisters, panel_holding);
        else
            err_frame_cnt++;

        tx_frame_cnt++;
    }

    if (m_numEntries.Input > 0)
    {
        std::expected<std::vector<uint16_t>, ModbusError> reg = m_Serial->ReadInputRegister(m_slaveId, m_numEntries.inputOffset, m_numEntries.Input);
        if(reg.has_value() && !reg->empty())
            HandleRegisterReading(*reg, m_Input, m_numEntries.Input, panel_inputReg);
        else
            err_frame_cnt++;
        /*
        std::expected<std::vector<uint16_t>, ModbusError> special_reg = m_Serial->ReadInputRegister(m_slaveId, 31000 - 1, EVENTLOG_BUFFER_SIZE + 1);
        if(special_reg && special_reg->size() == EVENTLOG_BUFFER_SIZE + 1)
        {
            if(!special_reg->empty())
            {
                size_t eventlog_cnt = special_reg->at(0);
                special_reg->erase(special_reg->begin());
                while(eventlog_cnt)
                {
                    std::vector<uint16_t> data = std::vector<uint16_t>(special_reg->begin(), special_reg->begin() + EVENTLOG_ENTRY_SIZE);
                    special_reg->erase(special_reg->begin(), special_reg->begin() + EVENTLOG_ENTRY_SIZE);

                    if(!data.empty())
                        m_EventLogEntries.push_back(std::make_unique<EventLogEntry>(data, std::chrono::steady_clock::now()));
                    eventlog_cnt--;
                }
                DBG("break");
            }
        }
        */
        tx_frame_cnt++;
    }
}

void ModbusEntryHandler::HandleWrites()
{
    for(auto& c : m_pendingCoilWrites)
    {
        std::expected<std::vector<uint8_t>, ModbusError> reg = m_Serial->ForceSingleCoil(m_slaveId, c.first, c.second);
        if(reg.has_value() && !reg->empty())
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
        if (m_Holding[c.first]->m_Type == ModbusBitfieldType::MBT_UI32)
        {
            vec.push_back(c.second & 0xFFFF);
            vec.push_back(c.second >> 16 & 0xFFFF);
        }
        else
        {
            vec.push_back(c.second & 0xFFFF);
        }

        size_t reg_offset = m_Holding[c.first]->m_Offset;
        std::expected<std::vector<uint8_t>, ModbusError> reg = m_Serial->WriteHoldingRegister(m_slaveId, m_numEntries.holdingOffset + reg_offset, vec.size(), vec);
        if(reg.has_value() && !reg->empty())
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
    while(!token.stop_requested())
    {
        WaitUntilInited(token);

        if (m_isMainThreadPaused)
        {
            m_isOpenInProgress = false;
            if (GetSerial().IsOpen() && !m_isCloseInProgress)
            {
                m_isCloseInProgress = true;
                try
                {
                    GetSerial().Close();
                }
                catch (boost::system::system_error&)
                {

                }
                m_isCloseInProgress = false;
            }
        }

        WaitIfPaused(token);
        
        if (is_enabled)
        {
            if (!GetSerial().IsOpen() && !m_isOpenInProgress)
            {
                m_isOpenInProgress = true;
                GetSerial().Open();
                m_isOpenInProgress = false;

                if (!GetSerial().IsOpen())
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }


            if (!token.stop_requested() && GetSerial().IsOpen())
            {
                HandlePolling();
                HandleWrites();

                {
                    std::unique_lock lock(m);
                    auto now = std::chrono::system_clock::now();
                    cv.wait_until(lock, token, now + std::chrono::milliseconds(m_pollingRate), []() { return 0 == 1; });
                }
            }
        }
    }
}