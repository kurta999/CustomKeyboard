#include "pch.hpp"

CanEntryHandler::CanEntryHandler(ICanEntryLoader& loader, ICanRxEntryLoader& rx_loader) :
    m_CanEntryLoader(loader), m_CanRxEntryLoader(rx_loader)
{
    start_time = std::chrono::steady_clock::now();
}

CanEntryHandler::~CanEntryHandler()
{
    to_exit = true;
    if(m_worker && m_worker->joinable())
    {
        m_worker->join();
        m_worker.reset(nullptr);
    }
}

XmlCanEntryLoader::~XmlCanEntryLoader()
{

}

XmlCanRxEntryLoader::~XmlCanRxEntryLoader()
{

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
            std::unique_ptr<CanTxEntry> local_entry = std::make_unique<CanTxEntry>();
            std::string frame_id = v.second.get_child("ID").get_value<std::string>();
            local_entry->id = std::stoi(frame_id, nullptr, 16);

            char bytes[128] = { 0 };
            std::string hex_str = v.second.get_child("Data").get_value<std::string>();
            boost::algorithm::erase_all(hex_str, " ");
            if(hex_str.length() > 16)
                hex_str.erase(16, hex_str.length() - 16);
            utils::ConvertHexStringToBuffer(hex_str, std::span{ bytes });
            local_entry->data.assign(bytes, bytes + (hex_str.length() / 2));
            local_entry->period = v.second.get_child("Period").get_value<int>();
            local_entry->comment = v.second.get_child("Comment").get_value<std::string>();
            e.push_back(std::move(local_entry));
        }
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlCanEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e)
{
    bool ret = true;
    std::ofstream out(path, std::ofstream::binary);
    if(out.is_open())
    {
        out << "<CanUsbXml>\n";
        for(auto& i : e)
        {
            out << "\t<Frame>\n";
            out << std::format("\t\t<ID>{:X}</ID>\n", i->id);
            std::string hex;
            utils::ConvertHexBufferToString(i->data, hex);
            out << std::format("\t\t<Data>{}</Data>\n", hex);
            out << std::format("\t\t<Period>{}</Period>\n", i->period);
            out << std::format("\t\t<Comment>{}</Comment>\n", i->comment);
            out << "\t</Frame>\n";
        }
        out << "</CanUsbXml>\n";
    }
    else
    {
        ret = false;
    }
    return ret;
}

bool XmlCanRxEntryLoader::Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("CanUsbRxXml")) /* loop over each Frame */
        {
            std::string frame_id = v.second.get_child("ID").get_value<std::string>();
            uint32_t id = std::stoi(frame_id, nullptr, 16);
            
            std::string comment = v.second.get_child("Comment").get_value<std::string>();
            e[id] = std::move(comment);
        }
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlCanRxEntryLoader::Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e)
{
    bool ret = true;
    std::ofstream out(path, std::ofstream::binary);
    if(out.is_open())
    {
        out << "<CanUsbRxXml>\n";
        for(auto& i : e)
        {
            out << "\t<Frame>\n";
            out << std::format("\t\t<ID>{:X}</ID>\n", i.first);
            out << std::format("\t\t<Comment>{}</Comment>\n", i.second);
            out << "\t</Frame>\n";
        }
        out << "</CanUsbRxXml>\n";
    }
    else
    {
        ret = false;
    }
    return ret;
}

void CanEntryHandler::Init()
{
    LoadTxList(default_tx_list);
    LoadRxList(default_rx_list);
    m_worker = std::make_unique<std::thread>([this] {
        while(!to_exit)
        {
            {
                std::scoped_lock lock{ m };
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
            }

            std::this_thread::sleep_for(1ms);
        }
        });
}

void CanEntryHandler::OnFrameSent(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    std::scoped_lock lock{ m };
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
                        m_LogEntries.push_back(std::make_unique<CanLogEntry>(CAN_LOG_DIR_TX, frame_id, data, data_len, i->last_execution));
                }
            }
            break;
        }
    }
    tx_frame_cnt++;
}

void CanEntryHandler::OnFrameReceived(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    std::scoped_lock lock{ m };
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    auto it = m_rxData.find(frame_id);
    if(it == m_rxData.end())
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
    rx_frame_cnt++;
    if(is_recoding)
        m_LogEntries.push_back(std::make_unique<CanLogEntry>(CAN_LOG_DIR_RX, frame_id, data, data_len, m_rxData[frame_id]->last_execution));
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
    m_LogEntries.clear();
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
    bool ret = m_CanRxEntryLoader.Load(path, rx_entry_comment);
    return ret;
}

bool CanEntryHandler::SaveRxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_rx_list;
    bool ret = m_CanRxEntryLoader.Save(path, rx_entry_comment);
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
            out << "Time,Direction,FrameID,DatSize,Data,Comment\n";
            for(auto& i : m_LogEntries)
            {
                std::string hex;
                utils::ConvertHexBufferToString(i->data, hex);
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(i->last_execution - start_time).count();
                out << std::format("{:.3},{},{:X},{},{},{}\n", static_cast<double>(elapsed) / 1000.0, i->direction == 0 ? "TX" : "RX", static_cast<uint32_t>(i->frame_id), i->data.size(),
                    hex, "");
            }
            out.flush();
            ret = true;
        }
    }

    if(ret)
    {
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        std::lock_guard lock(frame->mtx);
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::CanLogSaved), dif, path.generic_string()});
    }
    return ret;
}