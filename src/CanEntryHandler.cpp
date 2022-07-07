#include "pch.hpp"

using namespace std::chrono_literals;

CanEntryHandler::CanEntryHandler(ICanEntryLoader& loader, ICanRxEntryLoader& rx_loader) :
    m_CanEntryLoader(loader), m_CanRxEntryLoader(rx_loader)
{

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
            std::string hash;
            try
            {
                hash = boost::algorithm::unhex(hex_str);
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Exception with boost::algorithm::unhex, str: {}", hex_str);
            }
            std::copy(hash.begin(), hash.end(), bytes);

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
    std::ofstream out(path.generic_string(), std::ofstream::binary);
    out << "<CanUsbXml>\n";

    for(auto& i : e)
    {
        out << "\t<Frame>\n";
        out << std::format("\t\t<ID>{:X}</ID>\n", i->id);
        std::string hex;
        boost::algorithm::hex(i->data.begin(), i->data.end(), std::back_inserter(hex));
        if(hex.length() > 2)
            utils::separate<2, ' '>(hex);
        out << std::format("\t\t<Data>{}</Data>\n", hex);
        out << std::format("\t\t<Period>{}</Period>\n", i->period);
        out << std::format("\t\t<Comment>{}</Comment>\n", i->comment);
        out << "\t</Frame>\n";
    }
    out << "</CanUsbXml>\n";
    return true;
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
    std::ofstream out(path.generic_string(), std::ofstream::binary);
    out << "<CanUsbRxXml>\n";

    for(auto& i : e)
    {
        out << "\t<Frame>\n";
        out << std::format("\t\t<ID>{:X}</ID>\n", i.first);
        out << std::format("\t\t<Comment>{}</Comment>\n", i.second);
        out << "\t</Frame>\n";
    }
    out << "</CanUsbRxXml>\n";
    return true;
}

void CanEntryHandler::Init()
{
    LoadTxList(default_tx_list);
    LoadRxList(default_rx_list);
    t = std::make_unique<std::thread>([this] {
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
                            CanSerialPort::Get()->AddToTxQueue(i->id, i->data.size(), (uint8_t*)i->data.data());
                            i->single_shot = false;
                        }
                        else
                        {
                            uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - i->last_execution).count();
                            if(elapsed > i->period)
                            {
                                CanSerialPort::Get()->AddToTxQueue(i->id, i->data.size(), (uint8_t*)i->data.data());
                                i->last_execution = std::chrono::steady_clock::now();
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
            i->count++;
            MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
            frame->can_panel->can_grid_tx->UpdateTxCounter(frame_id, i->count);
            break;
        }
    }
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
        m_rxData[frame_id]->data.resize(data_len);
        m_rxData[frame_id]->data.assign(data, data + data_len);
        uint32_t elapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(time_now - m_rxData[frame_id]->last_execution).count());
        m_rxData[frame_id]->period = elapsed;
        m_rxData[frame_id]->count++;
    }
    m_rxData[frame_id]->last_execution = std::chrono::steady_clock::now();
}

CanEntryHandler::~CanEntryHandler()
{
    to_exit = true;
    if(t && t->joinable())
        t->join();
}

bool CanEntryHandler::LoadTxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_tx_list;
    
    entries.clear();
    bool ret = m_CanEntryLoader.Load(path, entries);
    return ret;
}

void CanEntryHandler::SaveTxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_tx_list;
    m_CanEntryLoader.Save(path, entries);
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

void CanEntryHandler::SaveRxList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = default_rx_list;
    m_CanRxEntryLoader.Save(path, rx_entry_comment);
}