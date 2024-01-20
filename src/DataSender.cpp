#include "pch.hpp"

constexpr int DATA_SIZE = 1024;

bool XmlDataEntryLoader::Load(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for(const boost::property_tree::ptree::value_type& v : pt.get_child("DataSenderXml")) /* loop over each Frame */
        {
            char bytes[DATA_SIZE] = { 0 };
            std::string hex_str = v.second.get_child("Data").get_value<std::string>();

            char bytes_response[DATA_SIZE] = { 0 };
            std::string response_hex_str = v.second.get_child("ExpectedResponse").get_value<std::string>();

            DataEntryType type;
            std::string data_type_str = v.second.get_child("Type").get_value<std::string>();

            size_t data_len = 0;
            size_t response_len = 0;
            if(data_type_str == "hex")
            {
                type = DataEntryType::Hex;

                boost::algorithm::erase_all(hex_str, " ");
                utils::ConvertHexStringToBuffer(hex_str, std::span{ bytes });

                boost::algorithm::erase_all(response_hex_str, " ");
                utils::ConvertHexStringToBuffer(response_hex_str, std::span{ bytes_response });

                data_len = (hex_str.length() / 2);
                response_len = (response_hex_str.length() / 2);
            }
            else if(data_type_str == "string")
            {
                type = DataEntryType::String;

                boost::algorithm::replace_all(hex_str, "\\r", "\r");  /* Fix for newlines */
                boost::algorithm::replace_all(hex_str, "\\n", "\n");  /* Fix for newlines */
                strncat(bytes, hex_str.c_str(), std::min(sizeof(bytes), hex_str.length()));
                strncat(bytes_response, response_hex_str.c_str(), std::min(sizeof(bytes_response), response_hex_str.length()));

                data_len = hex_str.length();
                response_len = response_hex_str.length();
            }
            else
            {
                LOG(LogLevel::Error, "Invalid data type: {}", data_type_str);
                continue;
            }

            DataEntrySendType send_type;
            std::string data_send_type_str = v.second.get_child("SendType").get_value<std::string>();
            if(data_send_type_str == "auto")
                send_type = DataEntrySendType::Auto;
            else if(data_send_type_str == "trigger")
                send_type = DataEntrySendType::Trigger;
            else
            {
                LOG(LogLevel::Error, "Invalid data send type: {}", data_send_type_str);
                continue;
            }

            std::string comment = v.second.get_child("Comment").get_value<std::string>();
            size_t step = v.second.get_child("Step").get_value<size_t>();
            uint32_t period = v.second.get_child("Period").get_value<uint32_t>();
            uint32_t response_timeout = v.second.get_child("ResponseTimeout").get_value<uint32_t>();

            std::unique_ptr<DataEntry> local_entry = std::make_unique<DataEntry>((uint8_t*)bytes, data_len, (uint8_t*)bytes_response, response_len, type, send_type,
                step, period, response_timeout, comment);
            local_entry->m_TextName = v.second.get_child("TextName").get_value<std::string>();
            local_entry->m_StartWith = v.second.get_child("StartWith").get_value<std::string>();

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

bool XmlDataEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("DataSender", boost::property_tree::ptree{});
    for(auto& i : e)
    {
        std::string hex;
        if(i->m_type == DataEntryType::Hex)
        {
            utils::ConvertHexBufferToString(i->data, hex);
        }
        else
        {
            for(auto& x : i->data)
				hex += x;
        }

        std::string response_hex;
        if(i->m_type == DataEntryType::Hex)
        {
            utils::ConvertHexBufferToString(i->response, response_hex);
        }
        else
        {
            for(auto& x : i->response)
                response_hex += x;
        }

        auto& frame_node = root_node.add_child("Data", boost::property_tree::ptree{});
        frame_node.add("Data", hex);
        frame_node.add("ExpectedResponse", response_hex);
        frame_node.add("Type", i->m_type == DataEntryType::Hex ? "hex" : "string");
        frame_node.add("SendType", i->m_sendType == DataEntrySendType::Auto ? "auto" : "trigger");
        frame_node.add("Comment", i->m_comment);
        frame_node.add("Step", std::format("{}", i->m_maxSendCount));
        frame_node.add("Period", std::format("{}", i->m_period));
        frame_node.add("ResponseTimeout", std::format("{}", i->m_responseTimeout));
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

DataSender::DataSender(IDataEntryLoader& loader) :
    m_DataEntryLoader(loader)
{
    start_time = std::chrono::steady_clock::now();
}

DataSender::~DataSender()
{
    {
        std::unique_lock lock{ m };
        m_cv.notify_all();
        m_RecvCv.notify_all();
    }

    m_worker.reset(nullptr);
}

void DataSender::Init()
{
    LoadFiles();
    SaveFiles();

    bool is_enabled = DataSerialPort::Get()->IsEnabled();
    if (is_enabled)
    {
        m_worker = std::make_unique<std::jthread>(std::bind_front(&DataSender::WorkerThread, this));
        if (m_worker)
            utils::SetThreadName(*m_worker, "DataEntryHandler");
    }
}

void DataSender::SetLogHelper(IDataLogHelper* helper)
{
    m_helper = helper;
}

void DataSender::LoadFiles()
{
    defaultDataList = "DataSender.xml";
    LoadDataList(defaultDataList);
}

void DataSender::SaveFiles()
{
    defaultDataList = "DataSender2.xml";
    SaveDataList(defaultDataList);
}

void DataSender::SendData(uint8_t* data, size_t data_size, DataEntryType type)
{
    m_response.clear();
    m_isResponseReceived = false;

    m_lastSentDataType = type;
    m_lastSentData = std::string((char*)data, (char*)data + data_size);
    tx_msg_cnt++;
    DataSerialPort::Get()->AddToTxQueue(data_size, data);
}

void DataSender::SendString(const char* str)
{
    size_t len = strlen(str);
    SendData((uint8_t*)str, len, DataEntryType::String);
}

void DataSender::SendString(const std::string& str)
{
    SendString(str.c_str());
}

void DataSender::HandleAutoSendFrames(std::unique_ptr<DataEntry>& entry, uint32_t& vecIndex, std::stop_token& token, std::unique_lock<std::mutex>& lock)
{
    std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();
    if (entry->m_period != 0)
    {
        if (entry->m_maxSendCount == 1)  /* Do not check time in case of singleshot */
        {
            SendData(entry->data.data(), entry->data.size(), entry->m_type);
            m_lastSentEntry = entry.get();

            vecIndex++;
            if (vecIndex >= entries.size())
                vecIndex = 0;
        }
        else
        {
            uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - entry->lastExecution).count();
            if (elapsed > entry->m_period)
            {
                entry->lastExecution = std::chrono::steady_clock::now();
                if (++entry->m_sentTimes >= entry->m_maxSendCount)
                {
                    entry->m_sentTimes = 0;
                    vecIndex++;
                    if (vecIndex >= entries.size())
                        vecIndex = 0;
                }

                SendData(entry->data.data(), entry->data.size(), entry->m_type);
                m_lastSentEntry = entry.get();
            }
        }

        //DataSerialPort::Get()->ProcessReceivedFrames();
        auto now = std::chrono::system_clock::now();
        bool ret = m_RecvCv.wait_until(lock, token, now + std::chrono::milliseconds(entry->m_responseTimeout), [this]() { return m_isResponseReceived.load(); });
        if (ret)
        {
            if (m_isResponseReceived)
            {
                if (!entry->response.empty())
                {
                    int cnt = 0;
                    for (auto& x : entry->response)
                    {
                        if (x == m_response[cnt])
                        {
                            cnt++;
                        }
                    }

                    if (cnt == m_response.length())  /* If every character matches */
                    {
                        std::string msg = std::format("RX Data OK - {}", entry->m_comment);
                        entry->m_LastResponse = m_response;

                        AddToLog(entry.get(), msg);
                    }
                    else
                    {
                        std::string msg = std::format("ERR: RX - Sent: {}, Recv: {}: {}", m_lastSentData, entry->m_comment, m_response);
                        AddToLog(entry.get(), msg);
                        m_error_count++;
                    }
                }
                else
                {
                    std::string msg = std::format("RX Data - {}", entry->m_comment);
                    entry->m_LastResponse = m_response;

                    AddToLog(entry.get(), msg);
                }
            }
            else
            {

                AddToLog(entry.get(), std::format("ERR: NO RESPONSE: {}", m_lastSentData));
                LOG(LogLevel::Normal, "ERR: NO RESPONSE : {}", m_lastSentData);
                m_error_count++;
            }
        }
        else
        {
            AddToLog(entry.get(), std::format("ERR: CV TIMEOUT - NO RESPONSE: {}", m_lastSentData));
            LOG(LogLevel::Normal, "ERR: CV TIMEOUT - NO RESPONSE : {}", m_lastSentData);
            m_error_count++;
        }
        m_isResponseReceived = false;
    }
}

void DataSender::HandleTriggerSendFrames(std::unique_ptr<DataEntry>& entry, uint32_t& vecIndex, std::stop_token& token, std::unique_lock<std::mutex>& lock)
{
    std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();
    if (entry->m_period != 0)
    {
        auto now = std::chrono::system_clock::now();
        bool ret = m_RecvCv.wait_until(lock, token, now + std::chrono::milliseconds(entry->m_responseTimeout), [this]() { return m_isResponseReceived.load(); });
        if (ret)
        {
            if (m_isResponseReceived)
            {
                if (!entry->response.empty())
                {
                    int cnt = 0;
                    for (auto& x : entry->response)
                    {
                        if (x == m_response[cnt])
                        {
                            cnt++;
                        }
                    }

                    if (cnt == m_response.length())  /* If every character matches */
                    {
                        std::string msg = std::format("RX Data OK - {}", entry->m_comment);
                        entry->m_LastResponse = m_response;

                        AddToLog(entry.get(), msg);
                    }
                    else
                    {
                        std::string msg = std::format("ERR: RX - Sent: {}, Recv: {}: {}", m_lastSentData, entry->m_comment, m_response);
                        AddToLog(entry.get(), msg);
                        m_error_count++;
                    }
                }
                else
                {
                    std::string msg = std::format("RX Data - {}", entry->m_comment);
                    entry->m_LastResponse = m_response;

                    AddToLog(entry.get(), msg);
                }

                if (entry->m_maxSendCount == 1)  /* Do not check time in case of singleshot */
                {
                    vecIndex++;
                    if (vecIndex >= entries.size())
                        vecIndex = 0;
                }
                else
                {
                    entry->lastExecution = std::chrono::steady_clock::now();
                    if (++entry->m_sentTimes >= entry->m_maxSendCount)
                    {
                        entry->m_sentTimes = 0;
                        vecIndex++;
                        if (vecIndex >= entries.size())
                            vecIndex = 0;
                    }
                }
                SendData(entry->data.data(), entry->data.size(), entry->m_type);
                m_lastSentEntry = entry.get();
            }
            else
            {

                AddToLog(entry.get(), std::format("ERR: NO RESPONSE: {}", m_lastSentData));
                LOG(LogLevel::Normal, "ERR: NO RESPONSE : {}", m_lastSentData);
                m_error_count++;
            }
        }
        else
        {
            AddToLog(entry.get(), std::format("ERR: CV TIMEOUT - NO RESPONSE: {}", m_lastSentData));
            LOG(LogLevel::Normal, "ERR: CV TIMEOUT - NO RESPONSE : {}", m_lastSentData);
            m_error_count++;
        }
        m_isResponseReceived = false;
    }
}

void DataSender::WorkerThread(std::stop_token token)
{
    std::unique_lock lock{ m };
    uint32_t vec_index = 0;
    while(!token.stop_requested())
    {
        if(!entries.empty() && is_sending)
        {
            auto& i = entries.at(vec_index);
            if (i->m_sendType == DataEntrySendType::Auto)
            {
				HandleAutoSendFrames(i, vec_index, token, lock);
			}
            else if (i->m_sendType == DataEntrySendType::Trigger)
            {
				HandleTriggerSendFrames(i, vec_index, token, lock);
			}

            if(vec_index >= entries.size())
                vec_index = 0;
        }
        m_cv.wait_for(lock, token, 1ms, []() { return 0 == 1; });
    }
    DBG("exit");
}

void DataSender::OnDataSent(size_t data_len, uint8_t* data)
{
    std::string hex;
    if(m_lastSentDataType == DataEntryType::Hex)
    {
        utils::ConvertHexBufferToString((const char*)data, data_len, hex);
    }
    else
    {
        hex = std::string((char*)data);
    }
    if(m_helper)
        m_helper->AppendLog(nullptr, std::format("TX: {}", hex));
}

void DataSender::OnDataReceived(size_t data_len, uint8_t* data)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    data[data_len] = 0;
    rx_msg_cnt++;

    std::string data_str = std::string(data, data + data_len);

    m_response.assign(data, data + data_len);
    m_isResponseReceived = true;
    m_isResponseProcessed = false;
    m_RecvCv.notify_all();
}

bool DataSender::LoadDataList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = defaultDataList;

    entries.clear();
    bool ret = m_DataEntryLoader.Load(path, entries);
    if(ret)
    {
        if(auto_send)
        {
            for(auto& i : entries)
            {
                i->m_send = true;
            }
        }

        is_recoding = auto_recording;  /* Toggle auto recording */
    }
    return ret;
}

bool DataSender::SaveDataList(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if(path.empty())
        path = defaultDataList;

    bool ret = m_DataEntryLoader.Save(path, entries);
    return ret;
}

void DataSender::ToggleAutoSend(bool toggle)
{
    auto_send = toggle;
}

void DataSender::ToggleRecording(bool toggle, bool is_pause)
{
    std::scoped_lock lock{ m };
    is_recoding = toggle;

    if(!is_pause && !toggle)
    {
        tx_msg_cnt = rx_msg_cnt = 0;
        //m_LogEntries.clear();
    }
}

void DataSender::On100msTimer()
{
    //std::unique_lock lock{ m };
    for (; !m_RecvMsg.empty(); m_RecvMsg.pop())
    {
        auto item = m_RecvMsg.back();
        if (m_helper)
        {
            if (item.second.starts_with("ERR"))
            {
                m_helper->OnError(m_error_count, item.second);
            }
            else
            {
                m_helper->AppendLog(item.first, item.second);
            }
        }
    }
}

void DataSender::IncreaseTxCounter()
{

}

void DataSender::ResetCounters()
{
    tx_msg_cnt = rx_msg_cnt = m_error_count = 0;
    m_lastSentData.clear();
}

void DataSender::AddToLog(DataEntry* entry, const std::string& msg)
{
    m_RecvMsg.push({entry, msg});
}