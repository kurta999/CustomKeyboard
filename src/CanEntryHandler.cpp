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
            std::string hash = boost::algorithm::unhex(hex_str);
            std::copy(hash.begin(), hash.end(), bytes);

            local_entry->data.insert(local_entry->data.end(), bytes, bytes + 8);
            local_entry->period = v.second.get_child("Period").get_value<int>();
            local_entry->comment = v.second.get_child("Comment").get_value<std::string>();
            e.push_back(std::move(local_entry));
        }
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOGMSG(error, "Exception thrown: {}, {}", e.filename(), e.what());
    }
    catch(std::exception& e)
    {
        LOGMSG(error, "Exception thrown: {}", e.what());
    }
    return true;
}

bool XmlCanEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e)
{
    std::ofstream out(path.generic_string(), std::ofstream::binary);
    out << "<CanUsbXml>\n";

    for(auto& i : e)
    {
        out << "\t<Frame>\n";
        out << fmt::format("\t\t<ID>{:X}</ID>\n", i->id);
        std::string hex;
        boost::algorithm::hex(i->data.begin(), i->data.end(), std::back_inserter(hex));
        utils::separate<2, ' '>(hex);
        out << fmt::format("\t\t<Data>{}</Data>\n", hex);
        out << fmt::format("\t\t<Period>{}</Period>\n", i->period);
        out << fmt::format("\t\t<Comment>{}</Comment>\n", i->comment);
        out << "\t</Frame>\n";
    }
    out << "</CanUsbXml>\n";
    return true;
}

bool XmlCanRxEntryLoader::Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e)
{
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("CanUsbRxXml")) /* loop over each Frame */
        {
            std::string frame_id = v.second.get_child("ID").get_value<std::string>();
            uint32_t id = std::stoi(frame_id, nullptr, 16);
            
            std::string comment = v.second.get_child("Data").get_value<std::string>();
            e[id] = std::move(comment);
        }
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOGMSG(error, "Exception thrown: {}, {}", e.filename(), e.what());
    }
    catch(std::exception& e)
    {
        LOGMSG(error, "Exception thrown: {}", e.what());
    }
    return true;
}

bool XmlCanRxEntryLoader::Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e)
{
    std::ofstream out(path.generic_string(), std::ofstream::binary);
    out << "<CanUsbRxXml>\n";

    for(auto& i : e)
    {
        out << "\t<Frame>\n";
        out << fmt::format("\t\t<ID>{:X}</ID>\n", i.first);
        out << fmt::format("\t\t<Comment>{}</Comment>\n", i.second);
        out << "\t</Frame>\n";
    }
    out << "</CanUsbRxXml>\n";
    return true;
}

void CanEntryHandler::Init()
{    
    LoadTxList(default_tx_list);
    t = std::make_unique<std::thread>([=] {
        while(!to_exit)
        {
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

            std::this_thread::sleep_for(10ms);
        }
        });
}

CanEntryHandler::~CanEntryHandler()
{
    to_exit = true;
    if(t && t->joinable())
        t->join();
}

void CanEntryHandler::LoadTxList(std::filesystem::path& path)
{
    if(path.empty())
        path = default_tx_list;
    
    entries.clear();
    m_CanEntryLoader.Load(path, entries);
    /* TODO: Update GUI */
}

void CanEntryHandler::SaveTxList(std::filesystem::path& path)
{
    if(path.empty())
        path = default_tx_list;
    m_CanEntryLoader.Save(path, entries);
}

void CanEntryHandler::LoadRxList(std::filesystem::path& path)
{
    if(path.empty())
        path = default_rx_list;

    rx_entry_comment.clear();
    m_CanRxEntryLoader.Load(path, rx_entry_comment);
    /* TODO: Update GUI */
}

void CanEntryHandler::SaveRxList(std::filesystem::path& path)
{
    if(path.empty())
        path = default_rx_list;
    m_CanRxEntryLoader.Save(path, rx_entry_comment);
}