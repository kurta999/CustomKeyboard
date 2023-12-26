#include "pch.hpp"

constexpr size_t TX_QUEUE_MAX_SIZE = 100;
constexpr size_t RX_CIRCBUFF_SIZE = 256;  /* Bytes */
constexpr auto DATA_SERIAL_PORT_TIMEOUT = 250ms;
constexpr auto DATA_SERIAL_PORT_EXCEPTION_TIMEOUT = 1000ms;
constexpr auto SEND_DELAY_BETWEEN_FRAMES = 500us;

DataSerialPort::DataSerialPort()
{

}

DataSerialPort::~DataSerialPort()
{
    std::unique_lock lock{ m_RxMutex };
    while (!m_TxQueue.empty())
    {
        m_TxQueue.pop();
    }
    m_cv.notify_all();
}

void DataSerialPort::Init()
{
    if(is_enabled)
    {
        auto recv_f = std::bind(&DataSerialPort::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2);
        auto send_f = std::bind(&DataSerialPort::OnDataSent, this, std::placeholders::_1);
        InitInternal("DataSerialPort", DATA_SERIAL_PORT_TIMEOUT, DATA_SERIAL_PORT_EXCEPTION_TIMEOUT, recv_f, send_f, m_Baudrate);
    }
    else
    {
        DeInitInternal();
    }
    /*
    std::string data = "AA BB CC DD EE    ";
    m_CircBuff.insert(m_CircBuff.end(), data.begin(), data.end());

    processReceivedFrames();
    std::string data2 = "FF GG HH\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\rA\r\rRTC_GET:7854456457455\rAT_ASD\r\r\rAT2\rAT3\rAT4\rAA BB CC\r";
    m_CircBuff.insert(m_CircBuff.end(), data2.begin(), data2.end());
    processReceivedFrames();
    printf("asd");*/
}

void DataSerialPort::AddToTxQueue(size_t data_len, uint8_t* data)
{
    if(!data || !data_len)
        return;
    std::unique_lock lock(m_mutex);
    m_TxQueue.push(std::make_unique<RawData>(data_len, data));

    if(m_TxQueue.size() > TX_QUEUE_MAX_SIZE)
    {
        LOG(LogLevel::Error, "Queue overflow");
        m_TxQueue.pop();
    }
    NotifiyMainThread();
}

void DataSerialPort::AddToRxQueue(size_t data_len, uint8_t* data)
{
    //std::unique_lock lock(m_mutex);
    std::unique_ptr<DataSender>& data_handler = wxGetApp().data_sender;
    if(data_handler)
        data_handler->OnDataReceived(data_len, data);
}

void DataSerialPort::OnDataReceived(const char* data, unsigned int len)
{
    std::scoped_lock guard(m_RxMutex);
    //m_CircBuff.insert(m_CircBuff.end(), data, data + len);

    m_CircBuff.insert(m_CircBuff.end(), data, data + len);
    ProcessReceivedFrames();

    NotifiyMainThread();
}

void DataSerialPort::ProcessReceivedFrames()
{
    int m_CircBuff_size = m_CircBuff.size();
    while(!m_CircBuff.empty())
    {
        bool b_continue = false;
        auto start_char = m_CircBuff.end();
        for(auto i = m_CircBuff.begin(); i != m_CircBuff.end() && m_CircBuff.size(); )
        {
            if (m_CircBuff.size() && *i == '\r')
            {
                if (start_char == m_CircBuff.end())
                {
                    i = m_CircBuff.erase(i);
                    start_char = m_CircBuff.end();
					continue;
				}

                size_t circbuff_size = m_CircBuff.size();
                std::string data_str;
                size_t data_len = i - start_char;
                if (data_len == std::numeric_limits<size_t>::max() || data_len > circbuff_size || circbuff_size == std::numeric_limits<size_t>::max())
                    break;
                std::copy_n(start_char, data_len, std::back_inserter(data_str));
                
                int dist = std::distance(start_char, i);
                if (dist < circbuff_size)
                {
                    if(dist == circbuff_size - 1)
                    {
                        m_CircBuff.clear();
                        i = m_CircBuff.end();
                        start_char = m_CircBuff.end();
                    }
                    else
                    {
                        i = m_CircBuff.erase(start_char, i);
                    }
                }
                else
                {
                    m_CircBuff.clear();
                    i = m_CircBuff.end();
                }

                start_char = m_CircBuff.end();
                if (data_len)
                {
                    //data[data_len] = 0;
                    DBG("%s - %lld\n", data_str.c_str(), data_str.length());

                    std::unique_ptr<DataSender>& data_handler = wxGetApp().data_sender;
                    if (data_handler)
                        data_handler->OnDataReceived(data_str.length(), (uint8_t*)data_str.c_str());

                    b_continue = true;
                }
                else
                {
                    b_continue = false;
                    break;
                }
            }
            else
            {
                if (!m_CircBuff.size())
                {
                    start_char = m_CircBuff.end();
                    break;
                }

                if(start_char == m_CircBuff.end())
                    start_char = i;
            }

            if (!m_CircBuff.size())
            {
                i = m_CircBuff.end();
                start_char = m_CircBuff.end();
                break;
            }
            
			if(i == m_CircBuff.end())
				break;
            else
            {
                if(m_CircBuff.size() < 2)
					break;
				else
					i++;
            }
        }

        if (!b_continue)
            break;
    }
}

void DataSerialPort::OnDataSent(CallbackAsyncSerial& serial_port)
{
    SendPendingDataFrames(serial_port);
}

void DataSerialPort::SendPendingDataFrames(CallbackAsyncSerial& serial_port)
{
    std::scoped_lock guard(m_RxMutex);
    while(!m_TxQueue.empty())
    {
        std::shared_ptr<RawData> data_ptr = m_TxQueue.front();

        {
            std::scoped_lock lock(m_mutex);
            if(data_ptr->data_len)
                serial_port.write((const char*)&data_ptr->data, data_ptr->data_len);
        }

        std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
        data_sender->OnDataSent(data_ptr->data_len, data_ptr->data);

        m_TxQueue.pop();

        std::this_thread::sleep_for(SEND_DELAY_BETWEEN_FRAMES);
    }
}

void DataSerialPort::ClearBuffers()
{
    //std::scoped_lock guard(m_RxMutex);
	m_CircBuff.clear();
    while (!m_TxQueue.empty())
    {
        m_TxQueue.pop();
	}
}