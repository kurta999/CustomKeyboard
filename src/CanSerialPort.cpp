#include "pch.hpp"

constexpr size_t TX_QUEUE_MAX_SIZE = 100;
constexpr size_t RX_CIRCBUFF_SIZE = 1024;  /* Bytes */
constexpr size_t CAN_SERIAL_TX_BUFFER_SIZE = 64;

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
        if(m_DeviceType == CanDeviceType::STM32)
            m_Device = std::make_unique<CanDeviceStm32>(m_CircBuff);
        else
            m_Device = std::make_unique<CanDeviceLawicel>(m_CircBuff);

        if(!m_worker)
            m_worker = std::make_unique<std::thread>(&CanSerialPort::WorkerThread, this);
    }
    else
    {
        DestroyWorkerThread();
    }
}

void CanSerialPort::SetDevice(std::unique_ptr<ICanDevice>&& device)
{
    m_Device = std::move(device);
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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

                m_Device->ProcessReceivedFrames();
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

void CanSerialPort::SendPendingCanFrames(CallbackAsyncSerial& serial_port)
{
    if(!m_TxQueue.empty())
    {
        std::shared_ptr<CanData> data_ptr = m_TxQueue.front();
        bool is_remove = false;

        {
            std::unique_lock lock(m_mutex);
            char data[CAN_SERIAL_TX_BUFFER_SIZE];
            size_t size = m_Device->PrepareSendDataFormat(data_ptr, data, sizeof(data), is_remove);

            if(size)
                serial_port.write((const char*)&data, size);
        }

        std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
        can_handler->OnFrameSent(data_ptr->frame_id, data_ptr->data_len, data_ptr->data);

        if(is_remove)
            m_TxQueue.pop();
    }
}