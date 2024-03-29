#include "pch.hpp"

CanEntryHandler::CanEntryHandler(ICanEntryLoader& loader, ICanRxEntryLoader& rx_loader, ICanMappingLoader& mapping_loader) :
    m_CanEntryLoader(loader), m_CanRxEntryLoader(rx_loader), m_CanMappingLoader(mapping_loader)
{
    start_time = std::chrono::steady_clock::now();
    isotp_init_link(&link, m_DefaultEcuId, m_Isotp_Sendbuf, sizeof(m_Isotp_Sendbuf), m_Isotp_Recvbuf, sizeof(m_Isotp_Recvbuf));
}

CanEntryHandler::~CanEntryHandler()
{
    {
        std::unique_lock lock{ m };
        m_cv.notify_all();
    }
    
    m_worker.reset(nullptr);
}

bool XmlCanEntryLoader::Load(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("CanUsbXml")) /* loop over each Frame */
        {
            std::string frame_id_str = v.second.get_child("ID").get_value<std::string>();
            uint32_t frame_id = 0;
            try
            {
                frame_id = std::stoi(frame_id_str, nullptr, 16);
            }
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "Invalid FrameID format, stoi exception: {} (FrameID: {})", e.what(), frame_id_str);
                continue;
            }

            auto frame_cnt = std::ranges::count(e, frame_id, &CanTxEntry::id);
            if(frame_cnt != 0)
            {
                LOG(LogLevel::Warning, "CAN frame with FrameID {} has been already added to the TX List, skipping this one", frame_id_str);
                continue;
            }

            char bytes[128] = { 0 };
            std::string hex_str = v.second.get_child("Data").get_value<std::string>();
            boost::algorithm::erase_all(hex_str, " ");
            if(hex_str.length() > 16)
                hex_str.erase(16, hex_str.length() - 16);
            utils::ConvertHexStringToBuffer(hex_str, std::span{ bytes });

            boost::optional<std::string> color;
            boost::optional<std::string> bg_color;
            boost::optional<bool> is_bold;
            boost::optional<float> is_scale;
            boost::optional<std::string> is_font_face;
            utils::xml::ReadChildIfexists<std::string>(v, "Color", color);
            utils::xml::ReadChildIfexists<std::string>(v, "BackgroundColor", bg_color);
            utils::xml::ReadChildIfexists<bool>(v, "Bold", is_bold);
            utils::xml::ReadChildIfexists<float>(v, "Scale", is_scale);
            utils::xml::ReadChildIfexists<std::string>(v, "FontFace", is_font_face);

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

            size_t data_len = (hex_str.length() / 2);
            std::unique_ptr<CanTxEntry> local_entry = std::make_unique<CanTxEntry>(frame_id, (uint8_t*)bytes, data_len, v.second.get_child("Period").get_value<int>(), v.second.get_child("LogLevel").get_value<uint8_t>(), v.second.get_child("Favourite").get_value<uint8_t>(), v.second.get_child("Comment").get_value<std::string>(), 
                color_, bg_color_, is_bold_, is_scale_, is_font_face_);
                
            e.push_back(std::move(local_entry));
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

bool XmlCanEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("CanUsbXml", boost::property_tree::ptree{});
    for(auto& i : e)
    {
        std::string hex;
        utils::ConvertHexBufferToString(i->data, hex);

        auto& frame_node = root_node.add_child("Frame", boost::property_tree::ptree{});
        frame_node.add("ID", std::format("{:X}", i->id));
        frame_node.add("Data", hex);
        frame_node.add("Period", i->period);
        frame_node.add("LogLevel", i->log_level);
        frame_node.add("Favourite", i->favourite_level);
        frame_node.add("Comment", i->comment);

        if(i->m_color)
            frame_node.add("Color", utils::ColorIntToString(*i->m_color));
        if(i->m_bg_color)
            frame_node.add("BackgroundColor", utils::ColorIntToString(*i->m_bg_color));
        if(i->m_is_bold)
            frame_node.add("Bold", "1");
        if(i->m_scale != 1.0f)
            frame_node.add("Scale", std::format("{:.1f}", i->m_scale));
        if(!i->m_font_face.empty())
            frame_node.add("FontFace", i->m_font_face);
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

bool XmlCanRxEntryLoader::Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e, std::unordered_map<uint32_t, uint8_t>& loglevels)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("CanUsbRxXml")) /* loop over each Frame */
        {
            std::string frame_id_str = v.second.get_child("ID").get_value<std::string>();
            uint32_t frame_id = 0;
            try
            {
                frame_id = std::stoi(frame_id_str, nullptr, 16);
            }
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "Invalid FrameID format, stoi exception: {} (FrameID: {})", e.what(), frame_id_str);
                continue;
            }

            auto frame_cnt = std::ranges::count(e, frame_id, [](const auto& item) { return item.first; });
            if(frame_cnt != 0)
            {
                LOG(LogLevel::Warning, "CAN frame with FrameID {} has been already added to the RX List, skipping this one", frame_id_str);
                continue;
            }

            std::string comment = v.second.get_child("Comment").get_value<std::string>();
            e[frame_id] = std::move(comment);
            loglevels[frame_id] = v.second.get_child("LogLevel").get_value<uint8_t>();
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

bool XmlCanRxEntryLoader::Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e, std::unordered_map<uint32_t, uint8_t>& loglevels) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("CanUsbRxXml", boost::property_tree::ptree{});
    for(auto& i : e)
    {
        auto& frame_node = root_node.add_child("Frame", boost::property_tree::ptree{});
        frame_node.add("ID", std::format("{:X}", i.first));
        frame_node.add("Comment", i.second);
        frame_node.add("LogLevel", loglevels[i.first]);
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

bool XmlCanMappingLoader::Load(const std::filesystem::path& path, CanMapping& mapping, CanFrameNameMapping& names, CanFrameSizeMapping& sizes, CanFrameDirectionMapping& directions)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml("FrameMapping.xml", pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("CanFrameMapping")) /* loop over each Frame */
        {
            std::string frame_id_str = v.second.get_child("ID").get_value<std::string>();
            std::string frame_name = v.second.get_child("Name").get_value<std::string>();
            uint32_t frame_id = 0;
            try
            {
                frame_id = std::stoi(frame_id_str, nullptr, 16);
            }
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "Invalid FrameID format, stoi exception: {} (FrameID: {})", e.what(), frame_id_str);
                continue;
            }

            if(frame_name.empty())
            {
                LOG(LogLevel::Warning, "Empty frame name for FrameID: {:X}", frame_id);
            }
            names[frame_id] = std::move(frame_name);
            sizes[frame_id] = v.second.get_child("Size").get_value<uint8_t>();
            directions[frame_id] = v.second.get_child("Direction").get_value<char>();

            uint8_t calculated_size = 0;
            for(const boost::property_tree::ptree::value_type& m : v.second) /* loop over each nested child */
            {
                if(m.first == "Mapping")
                {
                    uint8_t offset = m.second.get<uint8_t>("<xmlattr>.offset");
                    uint8_t len = m.second.get<uint8_t>("<xmlattr>.len");
                    std::string type = m.second.get<std::string>("<xmlattr>.type");
                    std::string name = m.second.get_value<std::string>();
                    int64_t min_val = std::numeric_limits<int64_t>::min();
                    int64_t max_val = std::numeric_limits<int64_t>::max();
                    uint32_t color = wxBLACK->GetRGB();
                    uint32_t bg_color = DEFAULT_TXTCTRL_BACKGROUND;
                    bool is_bold = false;
                    float scale = 1.0f;

                    CanBitfieldType bitfield_type = GetTypeFromString(type);
                    if(bitfield_type == CBT_INVALID)
                    {
                        LOG(LogLevel::Warning, "Invalid type used for frame mapping. FrameID: {:X}, type: {}", frame_id, type);
                        continue;
                    }

                    if(mapping.contains(frame_id))
                    {
                        auto offset_cnt = std::ranges::count(mapping[frame_id], offset, [](const auto& item) { return item.first; });
                        if(offset_cnt != 0)
                        {
                            LOG(LogLevel::Warning, "Duplicate offset for FrameID: {}, Name: {}, Offset: {} - skipping this one", frame_id_str, name, offset);
                            continue;
                        }
                    }

                    boost::optional<int64_t> min_val_child = m.second.get_optional<int64_t>("<xmlattr>.min");
                    boost::optional<int64_t> max_val_child = m.second.get_optional<int64_t>("<xmlattr>.max");
                    boost::optional<std::string> color_child = m.second.get_optional<std::string>("<xmlattr>.color");
                    boost::optional<std::string> bg_color_child = m.second.get_optional<std::string>("<xmlattr>.bg_color");
                    boost::optional<bool> is_bold_child = m.second.get_optional<bool>("<xmlattr>.bold");
                    boost::optional<float> scale_child = m.second.get_optional<float>("<xmlattr>.scale");

                    if(min_val_child)
                    {
                        min_val = *min_val_child;
                        if(min_val < GetMinMaxForType(bitfield_type).first)
                            min_val = GetMinMaxForType(bitfield_type).first;
                    }
                    else
                        min_val = GetMinMaxForType(bitfield_type).first;

                    if(max_val_child)
                    {
                        max_val = *max_val_child;
                        if(max_val > GetMinMaxForType(bitfield_type).second)
                            max_val = GetMinMaxForType(bitfield_type).second;
                    }
                    else
                        max_val = GetMinMaxForType(bitfield_type).second;

                    if(color_child)
                        color = utils::ColorStringToInt(*color_child);
                    if(bg_color_child)
                        bg_color = utils::ColorStringToInt(*bg_color_child);
                    if(is_bold_child)
                        is_bold = *is_bold_child;
                    if(scale_child)
                        scale = *scale_child;

                    std::string description;
                    boost::optional<std::string> description_child = m.second.get_optional<std::string>("<xmlattr>.desc");
                    if(description_child)
                    {
                        description = *description_child;
                        boost::algorithm::replace_all(description, "\\n", "\n");  /* Fix for newlines */
                    }

                    mapping[frame_id].try_emplace(offset, std::make_unique<CanMap>(std::move(name), bitfield_type, len, min_val, max_val, std::move(description),
                        color, bg_color, is_bold, scale));
                    calculated_size += len;
                }
            }
            
            if((calculated_size / 8) > sizes[frame_id])
            {
                LOG(LogLevel::Warning, "Calculated size for frame {} ({}) is bigger than predefined {}!", names[frame_id], calculated_size, sizes[frame_id]);
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

bool XmlCanMappingLoader::Save(const std::filesystem::path& path, CanMapping& mapping, CanFrameNameMapping& names, CanFrameSizeMapping& sizes, CanFrameDirectionMapping& directions) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("CanFrameMapping", boost::property_tree::ptree{});
    for(auto& m : mapping)
    {
        auto& frame_node = root_node.add_child("Frame", boost::property_tree::ptree{});
        frame_node.add("ID", std::format("{:X}", m.first));

        auto it_name = names.find(m.first);
        auto it_size = sizes.find(m.first);
        auto it_direction = directions.find(m.first);
        frame_node.add("Name", it_name != names.end() ? it_name->second : "");
        frame_node.add("Size", it_size != sizes.end() ? it_size->second : 0);
        frame_node.add("Direction", std::format("{:c}", it_direction != directions.end() ? it_direction->second : 'T'));

        for(auto& o : m.second)
        {
            std::string_view type_str = GetStringFromType(o.second->m_Type);
            auto& mapping_child = frame_node.add("Mapping", o.second->m_Name);
            mapping_child.put("<xmlattr>.offset", o.first);
            mapping_child.put("<xmlattr>.len", o.second->m_Size);
            mapping_child.put("<xmlattr>.type", type_str);
            mapping_child.put("<xmlattr>.min", o.second->m_MinVal);
            mapping_child.put("<xmlattr>.max", o.second->m_MaxVal);

            if(o.second->m_color != wxBLACK->GetRGB())
                mapping_child.put("<xmlattr>.color", utils::ColorIntToString(o.second->m_color));
            if(o.second->m_bg_color != DEFAULT_TXTCTRL_BACKGROUND)
                mapping_child.put("<xmlattr>.bg_color", utils::ColorIntToString(o.second->m_bg_color));
            if(o.second->m_is_bold)
                mapping_child.put("<xmlattr>.bold", true);
            if(o.second->m_scale != 1.0f)
                mapping_child.put("<xmlattr>.scale", o.second->m_scale);

            if(!o.second->m_Description.empty())
            {
                std::string desc_str = o.second->m_Description;
                boost::algorithm::replace_all(desc_str, "\n", "\\n");  /* Fix for newlines */
                mapping_child.put("<xmlattr>.desc", desc_str);
            }
        }
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

CanBitfieldType XmlCanMappingLoader::GetTypeFromString(const std::string_view& input)
{
    auto ret = std::find_if(m_CanBitfieldTypeMap.cbegin(), m_CanBitfieldTypeMap.cend(), [&input](const auto& item) { return item.second == input; });
    if(ret == m_CanBitfieldTypeMap.cend())
        return CanBitfieldType::CBT_INVALID;
    return ret->first;
}

const std::string_view XmlCanMappingLoader::GetStringFromType(CanBitfieldType type)
{
    auto it = m_CanBitfieldTypeMap.find(type);
    if(it != m_CanBitfieldTypeMap.end())
        return it->second;
    return m_CanBitfieldTypeMap[CBT_INVALID];
}

std::pair<int64_t, int64_t> XmlCanMappingLoader::GetMinMaxForType(CanBitfieldType type)
{
    auto it = m_CanTypeSizes.find(type);
    if(it != m_CanTypeSizes.end())
        return it->second;
    return m_CanTypeSizes[CBT_INVALID];
}

void CanEntryHandler::Init()
{
    LoadFiles();
    m_worker = std::make_unique<std::jthread>(std::bind_front(&CanEntryHandler::WorkerThread, this));
    if(m_worker)
        utils::SetThreadName(*m_worker, "CanEntryHandler");
}

void CanEntryHandler::LoadFiles()
{
    LoadTxList(default_tx_list);
    LoadRxList(default_rx_list);
    LoadMapping(default_mapping);
}

void CanEntryHandler::WorkerThread(std::stop_token token)
{
    while(!token.stop_requested())
    {
        {
            std::unique_lock lock{ m };
            std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
            for(auto& i : entries)
            {
                if((i->period != 0 && i->send) || i->single_shot)
                {
                    if(i->single_shot)  /* Do not check time in case of singleshot */
                    {
                        CanSerialPort::Get()->AddToTxQueue(i->id, i->data.size(), i->data.data());
                        i->single_shot = false;
                    }
                    else
                    {
                        uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - i->last_execution).count();
                        if(elapsed > i->period)
                        {
                            i->last_execution = std::chrono::steady_clock::now();
                            CanSerialPort::Get()->AddToTxQueue(i->id, i->data.size(), i->data.data());
                        }
                    }
                }
            }

            m_cv.wait_for(lock, token, 1ms, []() { return 0 == 1; });
            isotp_poll(&link);
        }
    }
    DBG("exit");
}

void CanEntryHandler::OnFrameSent(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    std::scoped_lock lock{ m };
    bool found = false;
    for(auto& i : entries)
    {
        if(i->id == frame_id)
        {
            if((MyFrame*)(wxGetApp().is_init_finished))
            {
                i->count++;
                MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
                if(frame && frame->is_initialized)
                {
                    frame->can_panel->sender->can_grid_tx->UpdateTxCounter(frame_id, i->count);
                    if(is_recoding)
                    {
                        if(i->log_level >= m_RecodingLogLevel)
                        {
                            if(i->period == 0 && !i->single_shot)
                            {
                                i->last_execution = std::chrono::steady_clock::now();
                            }

                            m_LogEntries.push_back(std::make_unique<CanLogEntry>(CAN_LOG_DIR_TX, frame_id, data, data_len, i->last_execution));
                        }
                    }
                }
            }
            found = true;
            break;
        }
    }

    if(!found && is_recoding) /* Append frame to log also if it's not defined in TX list */
    {
        std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        m_LogEntries.push_back(std::make_unique<CanLogEntry>(CAN_LOG_DIR_TX, frame_id, data, data_len, time_now));
    }

    NotifyFrameOnBus(frame_id, data, data_len);

    tx_frame_cnt++;
}

void CanEntryHandler::OnFrameReceived(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    std::scoped_lock lock{ m };
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();

    if(!m_rxData.contains(frame_id))
    {
        m_rxData[frame_id] = std::make_unique<CanRxData>(data, data_len);
    }
    else
    {
        m_rxData[frame_id]->data.assign(data, data + data_len);
        uint32_t elapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(time_now - m_rxData[frame_id]->last_execution).count());
        m_rxData[frame_id]->period = elapsed;
        m_rxData[frame_id]->count++;
    }
    m_rxData[frame_id]->last_execution = std::chrono::steady_clock::now();
    m_rxData[frame_id]->log_level = m_RxLogLevels.contains(frame_id) ? m_RxLogLevels[frame_id] : 1;  /* TODO: this is ugly and wastes resources as fuck, redesign it */
    rx_frame_cnt++;
    if(is_recoding)
    {
        if(m_rxData[frame_id]->log_level >= m_RecodingLogLevel)
            m_LogEntries.push_back(std::make_unique<CanLogEntry>(CAN_LOG_DIR_RX, frame_id, data, data_len, m_rxData[frame_id]->last_execution));
    }

    if(frame_id == m_IsoTpResponseId)
    {
        isotp_on_can_message(&link, data, data_len);
        
        uint16_t recv_size = 0;
        if(isotp_receive(&link, m_uds_recv_data, sizeof(m_uds_recv_data), &recv_size) == ISOTP_RET_OK)
        {
            DBG("iso-tp recv: %d", recv_size);
            m_UdsFrames.push_back(std::string((const char*)m_uds_recv_data, recv_size));
            last_uds_frame_received = std::chrono::steady_clock::now();

            NotifyIsoTpData(frame_id, m_uds_recv_data, recv_size);
        }
    }

    NotifyFrameOnBus(frame_id, data, data_len);

    m_cv.notify_all();
}

void CanEntryHandler::ToggleAutoSend(bool toggle)
{
    auto_send = toggle;
}

void CanEntryHandler::ToggleRecording(bool toggle, bool is_pause)
{
    std::scoped_lock lock{ m };
    is_recoding = toggle;

    if(!is_pause && !toggle)
    {
        tx_frame_cnt = rx_frame_cnt = 0;
        m_LogEntries.clear();
    }
}

void CanEntryHandler::ClearRecording()
{
    std::scoped_lock lock{ m };
    tx_frame_cnt = rx_frame_cnt = 0;
    m_LogEntries.clear();
}

void CanEntryHandler::SendDataFrame(uint32_t frame_id, uint8_t* data, uint16_t size)
{
    CanSerialPort::Get()->AddToTxQueue(frame_id, size, (uint8_t*)data);
}

void CanEntryHandler::SendIsoTpFrame(uint32_t frame_id, uint8_t* data, uint16_t size)
{
    isotp_send_with_id(&link, frame_id, data, size);
}

bool CanEntryHandler::LoadTxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_tx_list;
    
    entries.clear();
    bool ret = m_CanEntryLoader.Load(path, entries);
    if(ret)
    {
        if(auto_send)
        {
            for(auto& i : entries)
            {
                i->single_shot = false;
                i->send = true;
            }
        }

        is_recoding = auto_recording;  /* Toggle auto recording */
    }
    return ret;
}

bool CanEntryHandler::SaveTxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_tx_list;
    bool ret = m_CanEntryLoader.Save(path, entries);
    return ret;
}

bool CanEntryHandler::LoadRxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_rx_list;

    rx_entry_comment.clear();
    bool ret = m_CanRxEntryLoader.Load(path, rx_entry_comment, m_RxLogLevels);
    return ret;
}

bool CanEntryHandler::SaveRxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_rx_list;
    bool ret = m_CanRxEntryLoader.Save(path, rx_entry_comment, m_RxLogLevels);
    return ret;
}

bool CanEntryHandler::LoadMapping(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_mapping;

    m_mapping.clear();
    bool ret = m_CanMappingLoader.Load(path, m_mapping, m_frame_name_mapping, m_frame_size_mapping, m_frame_direction_mapping);
    return ret;
}

bool CanEntryHandler::SaveMapping(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_mapping;
    bool ret = m_CanMappingLoader.Save(path, m_mapping, m_frame_name_mapping, m_frame_size_mapping, m_frame_direction_mapping);
    return ret;
}

bool CanEntryHandler::SaveRecordingToFile(std::filesystem::path& path)
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::scoped_lock lock{ m };
    bool ret = false;
    if(!m_LogEntries.empty())
    {
        std::ofstream out(path, std::ofstream::binary);
        if(out.is_open())
        {
            out << "Time,Direction,FrameID,DataSize,Data,Comment\n";
            for(auto& i : m_LogEntries)
            {
                std::string hex;
                utils::ConvertHexBufferToString(i->data, hex);
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(i->last_execution - start_time).count();
                out << std::format("{:.3f},{},{:X},{},{}", static_cast<double>(elapsed) / 1000.0, i->direction == 0 ? "TX" : "RX", 
                    static_cast<uint32_t>(i->frame_id), i->data.size(), hex);

                std::string* comment = nullptr;
                if(i->direction == 0) /* TX */
                {
                    auto tx_entry_opt = FindTxCanEntryByFrame(i->frame_id);
                    if(tx_entry_opt.has_value())
                    {
                        comment = &tx_entry_opt->get().comment;
                    }
                }
                else  /* RX */
                {
                    if(rx_entry_comment.contains(i->frame_id))
                        comment = &rx_entry_comment[i->frame_id];
                }
                if(comment && !comment->empty())
                    out << "," << *comment << "\n";
                else
                    out << "\n";
            }
            out.flush();
            ret = true;
        }
        else
        {
            LOG(LogLevel::Error, "Failed to open file for saving CAN recording: {}", path.generic_string());
        }
    }

    if(ret)
    {
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        std::unique_lock lock(frame->mtx);
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::CanLogSaved), dif, path.generic_string()});
    }
    return ret;
}

void CanEntryHandler::GenerateLogForFrame(uint32_t frame_id, bool is_rx, std::vector<std::string>& log)
{
    if(!m_LogEntries.empty())
    {
        for(auto& i : m_LogEntries)
        {
            if(i->frame_id == frame_id)
            {
                std::string out;
                std::string hex;
                utils::ConvertHexBufferToString(i->data, hex);
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(i->last_execution - start_time).count();  /* Looks like there is a bug with fmt alingment */
                out = std::format("{:<6.03f}{:<6}{:<6X}{:<6}{:<6}", static_cast<double>(elapsed) / 1000.0, i->direction == 0 ? "TX" : "RX", 
                    static_cast<uint32_t>(i->frame_id), i->data.size(), hex);

                std::string* comment = nullptr;
                if(i->direction == 0) /* TX */
                {
                    auto tx_entry_opt = FindTxCanEntryByFrame(i->frame_id);
                    if(tx_entry_opt.has_value())
                    {
                        comment = &tx_entry_opt->get().comment;
                    }
                }
                else  /* RX */
                {
                    if(rx_entry_comment.contains(i->frame_id))
                        comment = &rx_entry_comment[i->frame_id];
                }
                if(comment && !comment->empty())
                    out += std::format("   {:^6}", *comment);

                log.push_back(std::move(out));
            }
        }
    }
}

template <typename T> void CanEntryHandler::HandleBitReading(uint32_t frame_id, bool is_rx, std::unique_ptr<CanMap>& m, size_t offset, CanBitfieldInfo& info)
{
    if(is_rx)
    {
        if(m_rxData.contains(frame_id))
        {
            uint64_t value = get_bitfield(m_rxData[frame_id]->data.data(), m_rxData[frame_id]->data.size(), offset, m->m_Size);
            T extracted_data = static_cast<T>(value);
            info.push_back({
                std::format("{}         (offset: {}, size: {}, range: {} - {})", m->m_Name, offset, m->m_Size, m->m_MinVal, m->m_MaxVal), std::to_string(extracted_data), m.get() });
        }
    }
    else
    {
        auto tx_entry_opt = FindTxCanEntryByFrame(frame_id);
        if(tx_entry_opt.has_value())
        {
            CanTxEntry& tx_entry = tx_entry_opt->get();
            uint64_t value = get_bitfield(tx_entry.data.data(), tx_entry.data.size(), offset, m->m_Size);
            T extracted_data = static_cast<T>(value);
            info.push_back({ 
                std::format("{}         (offset: {}, size: {}, range: {} - {})", m->m_Name, offset, m->m_Size, m->m_MinVal, m->m_MaxVal), std::to_string(extracted_data), m.get()});
        }
    }
}

template <typename T> void CanEntryHandler::HandleBitWriting(uint32_t frame_id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::vector<std::string>& new_data)
{
    T raw_data;
    try
    {
        raw_data = static_cast<T>(std::stoi(new_data[pos]));
        set_bitfield(raw_data, offset, size, byte_array, sizeof(byte_array));
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid input for pos {}. Exception: {}", pos, e.what());
    }

    //DBG("%s - %d, %x ......... %d\n", m.second->m_Name.c_str(), m.first, raw_data, m.second->m_Size);
    pos++;
}

CanBitfieldInfo CanEntryHandler::GetMapForFrameId(uint32_t frame_id, bool is_rx)
{
    CanBitfieldInfo info;
    if(m_mapping.contains(frame_id))
    {
        for(auto &[offset, m] : m_mapping[frame_id])
        {
            DBG("%d, %s\n", offset, m->m_Name.c_str());
            switch(m->m_Type)
            {
                case CBT_BOOL:
                case CBT_UI8:
                {
                    HandleBitReading<uint8_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_I8:
                {
                    HandleBitReading<int8_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_UI16:
                {
                    HandleBitReading<uint16_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_I16:
                {
                    HandleBitReading<int16_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_UI32:
                {
                    HandleBitReading<uint32_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_I32:
                {
                    HandleBitReading<int32_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_UI64:
                {
                    HandleBitReading<uint64_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_I64:
                {
                    HandleBitReading<int64_t>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_FLOAT:
                {
                    HandleBitReading<float>(frame_id, is_rx, m, offset, info);
                    break;
                }
                case CBT_DOUBLE:
                {
                    HandleBitReading<double>(frame_id, is_rx, m, offset, info);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    //DBG("\n\n%s\n\n", ret.c_str());
    return info;
}

void CanEntryHandler::ApplyEditingOnFrameId(uint32_t frame_id, std::vector<std::string> new_data)
{
    uint8_t cnt = 0;
    uint8_t byte_array[8] = {};
    auto tx_entry_opt = FindTxCanEntryByFrame(frame_id);
    if(tx_entry_opt.has_value())  /* Copy CAN frame's data to temporary byte_array */
        memcpy(byte_array, tx_entry_opt->get().data.data(), tx_entry_opt->get().data.size());

    if(m_mapping.contains(frame_id))
    {
        LOG(LogLevel::Normal, "MapSize: {}, NewDataSize: {}", m_mapping[frame_id].size(), new_data.size());
        for(auto& [offset, m] : m_mapping[frame_id])
        {
            switch(m->m_Type)
            {
                case CBT_BOOL:
                case CBT_UI8:
                {
                    HandleBitWriting<uint8_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I8:
                {
                    HandleBitWriting<int8_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_UI16:
                {
                    HandleBitWriting<uint16_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }                
                case CBT_I16:
                {
                    HandleBitWriting<int16_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }                
                case CBT_UI32:
                {
                    HandleBitWriting<uint32_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }                
                case CBT_I32:
                {
                    HandleBitWriting<int32_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }                
                case CBT_UI64:
                {
                    HandleBitWriting<uint64_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I64:
                {
                    HandleBitWriting<int64_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
            }
        }
        AssignNewBufferToTxEntry(frame_id, byte_array, sizeof(byte_array));
    }
    DBG("ok");
}

void CanEntryHandler::AssignNewBufferToTxEntry(uint32_t frame_id, uint8_t* buffer, size_t size)
{
    auto tx_entry = FindTxCanEntryByFrame(frame_id);
    if(tx_entry.has_value())
    {
        tx_entry->get().data.clear();
        tx_entry->get().data.assign(buffer, buffer + size);
    }
}

std::optional<std::reference_wrapper<CanTxEntry>> CanEntryHandler::FindTxCanEntryByFrame(uint32_t frame_id)
{
    auto ret = std::find_if(entries.cbegin(), entries.cend(), [&frame_id](const auto& item) { return item->id == frame_id; });
    if(ret == entries.cend())
        return {};
    return **ret;
}

uint32_t CanEntryHandler::FindFrameIdOnMapByName(const std::string& name)
{
#if 0
    uint32_t ret_frame_id = 0;
    for(auto& i : m_mapping)
    {
        for(auto& x : i.second)
        {
            if(boost::icontains(x.second->m_Name, name))  //if(x.second->m_Name == name)
            {
                return ret_frame_id;
            }
        }
    }
#endif

    uint32_t ret = 0;
    for(auto& i : m_frame_name_mapping)
    {
        if(i.second == name)
        {
            ret = i.first;
            break;
        }
    }
    return ret;
}

uint32_t CanEntryHandler::GetElapsedTimeSinceLastUdsFrame() const
{
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - last_uds_frame_received).count();
    return diff;
}

extern "C" void isotp_user_debug(const char* message, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);

    LOG(LogLevel::Verbose, "IsoTP: {}", buffer);
    DBG(buffer);
    //do something with the error

    va_end(args);
}


/* user implemented, get millisecond */
extern "C" uint32_t isotp_user_get_ms(void)
{
    using namespace utils;
    return GetTickCount();
}

extern "C" int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t * data, const uint8_t size)
{
    CanSerialPort::Get()->AddToTxQueue(arbitration_id, size, (uint8_t*)data);
    return 0;
}