#include "pch.hpp"

// PARAM_SET:P1,FC:100,L:1,BL:2,CL:3,SI:4,DB:5,NV:6,Code:7,DS:8,TS:9,UP10:10

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
    m_worker = std::make_unique<std::jthread>(std::bind_front(&DataSender::WorkerThread, this));
    if(m_worker)
        utils::SetThreadName(*m_worker, "DataEntryHandler");
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

void DataSender::SendData(uint8_t* data, size_t data_size, dataEntryType type)
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
    SendData((uint8_t*)str, len, dataEntryType::String);
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
                        AddToLog(msg);
                    }
                    else
                    {
                        std::string msg = std::format("ERR: RX - Sent: {}, Recv: {}: {}", m_lastSentData, entry->m_comment, m_response);
                        AddToLog(msg);
                        m_error_count++;
                    }
                }
                else
                {
                    std::string msg = std::format("RX Data - {}", entry->m_comment);
                    AddToLog(msg);
                }
            }
            else
            {

                AddToLog(std::format("ERR: NO RESPONSE: {}", m_lastSentData));
                LOG(LogLevel::Normal, "ERR: NO RESPONSE : {}", m_lastSentData);
                m_error_count++;
            }
        }
        else
        {
            AddToLog(std::format("ERR: CV TIMEOUT - NO RESPONSE: {}", m_lastSentData));
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
                        AddToLog(msg);
                    }
                    else
                    {
                        std::string msg = std::format("ERR: RX - Sent: {}, Recv: {}: {}", m_lastSentData, entry->m_comment, m_response);
                        AddToLog(msg);
                        m_error_count++;
                    }
                }
                else
                {
                    std::string msg = std::format("RX Data - {}", entry->m_comment);
                    AddToLog(msg);
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
            }
            else
            {

                AddToLog(std::format("ERR: NO RESPONSE: {}", m_lastSentData));
                LOG(LogLevel::Normal, "ERR: NO RESPONSE : {}", m_lastSentData);
                m_error_count++;
            }
        }
        else
        {
            AddToLog(std::format("ERR: CV TIMEOUT - NO RESPONSE: {}", m_lastSentData));
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
            if (i->m_sendType == dataEntrySendType::Auto)
            {
				HandleAutoSendFrames(i, vec_index, token, lock);
			}
            else if (i->m_sendType == dataEntrySendType::Trigger)
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
    if(m_lastSentDataType == dataEntryType::Hex)
    {
        utils::ConvertHexBufferToString((const char*)data, data_len, hex);
    }
    else
    {
        hex = std::string((char*)data);
    }
    if(m_helper)
        m_helper->AppendLog(std::format("TX: {}", hex));
}

void DataSender::OnDataReceived(size_t data_len, uint8_t* data)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    data[data_len] = 0;
    rx_msg_cnt++;

    std::string data_str = std::string(data, data + data_len);

    m_response.assign(data, data + data_len);
    m_isResponseReceived = true;
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
            if (item.starts_with("ERR"))
            {
                m_helper->OnError(m_error_count, item);
            }
            else
            {
                m_helper->AppendLog(item);
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

void DataSender::AddToLog(const std::string& msg)
{
    m_RecvMsg.push(msg);
}