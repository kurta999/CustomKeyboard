#include "pch.hpp"

constexpr size_t TX_QUEUE_MAX_SIZE = 100;
constexpr size_t RX_CIRCBUFF_SIZE = 1024;  /* Bytes */
constexpr size_t CAN_SERIAL_TX_BUFFER_SIZE = 64;
constexpr auto CAN_SERIAL_PORT_TIMEOUT = 5000ms;

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
        {
            m_worker = std::make_unique<std::jthread>(std::bind_front(&CanSerialPort::WorkerThread, this));
            utils::SetThreadName(*m_worker, "CanSerialPort");
        }
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
    {
        //LOG(LogLevel::Error, "Queue overflow");
        m_TxQueue.pop();
    }
    NotifiyMainThread();
}

void CanSerialPort::AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    //std::unique_lock lock(m_mutex);
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    if(can_handler)
        can_handler->OnFrameReceived(frame_id, data_len, data);
}

void CanSerialPort::NotifiyMainThread()
{
    is_notification_pending = true;
    m_cv.notify_all();
}

void CanSerialPort::DestroyWorkerThread()
{
    if(m_worker)
    {
        NotifiyMainThread();
        m_worker.reset(nullptr);
    }
}

void CanSerialPort::WorkerThread(std::stop_token token)
{
    while(!token.stop_requested())
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

            while(!token.stop_requested())
            {
                if(serial.errorStatus() || serial.isOpen() == false)
                {
                    LOG(LogLevel::Error, "Serial port can unexpectedly closed");
                    break;
                }

                {
                    std::unique_lock lock(m_mutex);
                    auto now = std::chrono::system_clock::now();
                    bool ret = m_cv.wait_until(lock, token, now + CAN_SERIAL_PORT_TIMEOUT, [this]() { return is_notification_pending != 0; });
                    if(!ret)
                    {
                        LOG(LogLevel::Error, "CV timeout");
                    }
                }

                m_Device->ProcessReceivedFrames(m_RxMutex);

                SendPendingCanFrames(serial);
                is_notification_pending = false;
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
            m_cv.wait_for(lock, token, 1000ms, []() { return 1 == 0; });
        }
    }
}

void CanSerialPort::OnDataReceived(const char* data, unsigned int len)
{
    std::scoped_lock guard(m_RxMutex);
    m_CircBuff.insert(m_CircBuff.end(), data, data + len);
    NotifiyMainThread();
}

void CanSerialPort::SendPendingCanFrames(CallbackAsyncSerial& serial_port)
{
    //DBG("txssize: %lld\n", m_TxQueue.size());
    while(!m_TxQueue.empty())
    {
        std::shared_ptr<CanData> data_ptr = m_TxQueue.front();
        bool is_remove = false;

        {
            std::scoped_lock lock(m_mutex);
            char data[CAN_SERIAL_TX_BUFFER_SIZE];
            size_t size = m_Device->PrepareSendDataFormat(data_ptr, data, sizeof(data), is_remove);

            if(size)
                serial_port.write((const char*)&data, size);
        }

        std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
        can_handler->OnFrameSent(data_ptr->frame_id, data_ptr->data_len, data_ptr->data);

        if(is_remove)
            m_TxQueue.pop();

        std::this_thread::sleep_for(std::chrono::microseconds(100));  /* This delay is needed because UART timeout won't happen if everything is sent at once */
    }
}