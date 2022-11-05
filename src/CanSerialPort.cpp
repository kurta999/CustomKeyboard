#include "pch.hpp"

constexpr uint32_t MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS = 0xAABBCCDD;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS = 0xAABBCCDE;

constexpr uint32_t TX_QUEUE_MAX_SIZE = 100;

constexpr uint32_t RX_CIRCBUFF_SIZE = 1024;  /* Bytes */

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic_number; // 0xAABBCCDD
    uint32_t frame_id;
    uint8_t data_len;
    uint8_t data[MAX_CAN_FRAME_DATA_LEN];
    uint16_t crc;
} UartCanData;
#pragma pack(pop)

CanSerialPort::CanSerialPort() : m_CircBuff(RX_CIRCBUFF_SIZE)
{
    
}

CanSerialPort::~CanSerialPort()
{
    DestroyWorkerThread();
}

void CanSerialPort::Init()
{
    if(is_enabled)
    {
        if(!m_worker)
            m_worker = std::make_unique<std::thread>(&CanSerialPort::WorkerThread, this);
    }
    else
    {
        DestroyWorkerThread();
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
    std::unique_lock lock(m_mutex);
    m_TxQueue.push(std::make_unique<CanData>(frame_id, data_len, data));

    if(m_TxQueue.size() > TX_QUEUE_MAX_SIZE)
        m_TxQueue.pop();
}

void CanSerialPort::AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    //std::unique_lock lock(m_mutex);
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    can_handler->OnFrameReceived(frame_id, data_len, data);
}

void CanSerialPort::DestroyWorkerThread()
{
    if(m_worker)
    {
        {
            std::lock_guard guard(m_mutex);
            to_exit = true;
            m_cv.notify_all();
        }
        if(m_worker->joinable())
            m_worker->join();
        m_worker.reset(nullptr);
    }
}

void CanSerialPort::WorkerThread()
{
    while(!to_exit)
    {
        std::string err_msg;
        try
        {
#ifdef _WIN32
            CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 921600);
#else
            CallbackAsyncSerial serial("/dev/ttyUSB" + std::to_string(com_port), 921600);
#endif
            serial.setCallback(std::bind(&CanSerialPort::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2));

            while(!to_exit)
            {
                if(serial.errorStatus() || serial.isOpen() == false)
                {
                    LOG(LogLevel::Error, "Serial port can unexpectedly closed");
                    break;
                }

                ProcessReceivedFrames();
                SendPendingCanFrames(serial);

                {
                    std::unique_lock lock(m_mutex);
                    m_cv.wait_for(lock, 5ms);
                }
            }
            try
            {
                serial.close();
            }
            catch(...)
            {

            }
        }
        catch(std::exception& e)
        {
            err_msg = e.what();
        }

        if(!err_msg.empty())
        {
            LOG(LogLevel::Error, "Exception CAN serial {}", err_msg);
            err_msg.clear();

            std::unique_lock lock(m_mutex);
            m_cv.wait_for(lock, 1000ms);
        }
    }
}

void CanSerialPort::OnDataReceived(const char* data, unsigned int len)
{
    std::lock_guard guard(m_RxMutex);
    m_CircBuff.insert(m_CircBuff.end(), data, data + len);
}

void CanSerialPort::ProcessReceivedFrames()
{
    std::lock_guard guard(m_RxMutex);
    while(m_CircBuff.size() >= sizeof(UartCanData))
    {
        do
        {
            char uart_data[sizeof(UartCanData)];
            std::copy(m_CircBuff.begin(), m_CircBuff.begin() + sizeof(UartCanData), uart_data);

            if(*(uint32_t*)&uart_data == MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS)
            {
                UartCanData* d = (UartCanData*)uart_data;
                uint16_t crc = utils::crc16_modbus((void*)uart_data, sizeof(UartCanData) - 2);
                if(crc == d->crc)
                {
                    AddToRxQueue(d->frame_id, d->data_len, d->data);
                }
                else
                {
                    std::string hex;
                    LOG(LogLevel::Verbose, "CRC mismatch, recv - calculated: {:X} != {:X}", d->crc, crc);
                    utils::ConvertHexBufferToString(reinterpret_cast<const char*>(d->data), sizeof(d->data), hex);
                    LOG(LogLevel::Verbose, "MagicNumber: {:X}, FrameID: {:X}, DataLen: {}", d->magic_number, d->frame_id, d->data_len);
                    hex.clear();
                    utils::ConvertHexBufferToString(uart_data, sizeof(uart_data), hex);
                    LOG(LogLevel::Verbose, "Full Data buffer: {}", hex);
                }
                m_CircBuff.erase(m_CircBuff.begin(), m_CircBuff.begin() + sizeof(UartCanData));
            }
            else
            {
                LOG(LogLevel::Verbose, "Invalid magic number: {:X}", *(uint32_t*)&uart_data);
                m_CircBuff.erase(m_CircBuff.begin());
            }
        } while(m_CircBuff.size() >= sizeof(UartCanData));  /* This second nested loop is needed for recovering when invalid data is received */
    }
}

void CanSerialPort::SendPendingCanFrames(CallbackAsyncSerial& serial_port)
{
    if(!m_TxQueue.empty())
    {
        std::shared_ptr<CanData> data_ptr = m_TxQueue.front();

        UartCanData d;
        d.magic_number = MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS;
        d.frame_id = data_ptr->frame_id;
        d.data_len = data_ptr->data_len;
        memcpy(d.data, data_ptr->data, d.data_len);
        d.crc = utils::crc16_modbus((void*)&d, sizeof(d) - 2);

        serial_port.write((const char*)&d, sizeof(UartCanData));

        CanEntryHandler* can_handler = wxGetApp().can_entry;
        can_handler->OnFrameSent(d.frame_id, d.data_len, d.data);
        m_TxQueue.pop();
    }
}