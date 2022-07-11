#include "pch.hpp"

using namespace std::chrono_literals;
using crc16_modbus_t = boost::crc_optimal<16, 0x8005, 0xFFFF, 0, true, true>;

constexpr uint32_t MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS = 0xAABBCCDD;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS = 0xAABBCCDE;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_ERR = 0xAABBCCDF;

constexpr uint32_t RX_QUEUE_MAX_SIZE = 1000;

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic_number; // 0xAABBCCDD
    uint32_t frame_id;
    uint8_t data_len;
    uint8_t data[8];
    uint16_t crc;
} UartCanData;
#pragma pack(pop)

CanSerialPort::~CanSerialPort()
{
    
}

void CanSerialPort::Init()
{
    if(is_enabled)
    {
        if(!m_worker)
            m_worker = std::make_unique<std::jthread>(std::bind(&CanSerialPort::UartReceiveThread, this, std::placeholders::_1, std::ref(m_cv), std::ref(m_mutex)));
    }
    else
    {
        if(m_worker)
            m_worker->request_stop();
    }
}

void CanSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    if(len != sizeof(UartCanData))
        return;
    UartCanData* d = (UartCanData*)data;
    crc16_modbus_t calc_result;
    calc_result.process_bytes((void*)data, len - 2);
    uint16_t crc = calc_result.checksum();
    if(crc == d->crc)
    {
        if(d->magic_number == MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS)
        {
            AddToRxQueue(d->frame_id, d->data_len, d->data);
        }
        else if(d->magic_number == MAGIC_NUMBER_RECV_DATA_ERR)
        {
            if(!m_TxQueue.empty())
                m_TxQueue.pop_front();
        }
    }
}

void CanSerialPort::UartReceiveThread(std::stop_token stop_token, std::condition_variable_any& cv, std::mutex& m)
{
    while(!stop_token.stop_requested())
    {
        try
        {
#ifdef _WIN32
            CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 921600);
#else
            CallbackAsyncSerial serial("/dev/ttyUSB" + std::to_string(com_port), 921600);
#endif
            serial.setCallback(std::bind(&CanSerialPort::OnUartDataReceived, this, std::placeholders::_1, std::placeholders::_2));

            while(!stop_token.stop_requested())
            {
                if(serial.errorStatus() || serial.isOpen() == false)
                {
                    LOG(LogLevel::Error, "Serial port can unexpectedly closed");
                    break;
                }

                if(!m_TxQueue.empty())
                {
                    std::shared_ptr<CanData> data_ptr = m_TxQueue.front();

                    UartCanData d;
                    d.magic_number = MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS;
                    d.frame_id = data_ptr->frame_id;
                    d.data_len = data_ptr->data_len;
                    memcpy(d.data, data_ptr->data, d.data_len);
                    
                    crc16_modbus_t calc_result;
                    calc_result.process_bytes((void*)&d, sizeof(d) - 2);

                    d.crc = calc_result.checksum();

                    serial.write((const char*)&d, sizeof(UartCanData));

                    CanEntryHandler* can_handler = wxGetApp().can_entry;
                    can_handler->OnFrameSent(d.frame_id, d.data_len, d.data);
                    m_TxQueue.pop_front();
                }

                std::unique_lock lock(m_mutex);
                std::stop_callback stop_wait{ stop_token, [&cv]() { cv.notify_one(); } };
                cv.wait_for(lock, 10ms, [&stop_token]() { return stop_token.stop_requested(); });
            }
            serial.close();
        }
        catch(std::exception& e)
        {
            LOG(LogLevel::Error, "Exception CAN serial: {}", e.what());
            {
                std::unique_lock lock(m_mutex);
                std::stop_callback stop_wait{ stop_token, [&cv]() { cv.notify_one(); } };
                cv.wait_for(lock, 1000ms, [&stop_token]() { return stop_token.stop_requested(); });
            }
        }
    }
}

void CanSerialPort::SetEnabled(bool enable)
{
    is_enabled = enable;
}

bool CanSerialPort::IsEnabled()
{
    return is_enabled;
}

void CanSerialPort::SetComPort(uint16_t port)
{
    com_port = port;
}

uint16_t CanSerialPort::GetComPort()
{
    return com_port;
}

void CanSerialPort::AddToTxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    if(!data || !data_len)
        return;
    m_TxQueue.push_back(std::make_unique<CanData>(frame_id, data_len, data));
}

void CanSerialPort::AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    // Update gui
    m_RxQueue.push_back(std::make_unique<CanData>(frame_id, data_len, data));
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    can_handler->OnFrameReceived(frame_id, data_len, data);

    if(m_RxQueue.size() > RX_QUEUE_MAX_SIZE)  /* TODO: once logging is added, move this to settings.ini */
        m_RxQueue.pop_back();
}