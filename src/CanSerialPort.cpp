#include "pch.hpp"

constexpr size_t TX_QUEUE_MAX_SIZE = 100;
constexpr size_t RX_CIRCBUFF_SIZE = 1024;  /* Bytes */
constexpr size_t CAN_SERIAL_TX_BUFFER_SIZE = 64;
constexpr auto CAN_SERIAL_PORT_TIMEOUT = 5000ms;
constexpr auto SEND_DELAY_BETWEEN_FRAMES = 100us;

CanSerialPort::CanSerialPort() : m_CircBuff(RX_CIRCBUFF_SIZE)
{
    auto recv_f = std::bind(&CanSerialPort::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2);
    auto send_f = std::bind(&CanSerialPort::OnDataSent, this, std::placeholders::_1);
    InitInternal("CanSerialPort", recv_f, send_f);
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

void CanSerialPort::OnDataReceived(const char* data, unsigned int len)
{
    std::scoped_lock guard(m_RxMutex);
    m_CircBuff.insert(m_CircBuff.end(), data, data + len);
    NotifiyMainThread();
}

void CanSerialPort::OnDataSent(CallbackAsyncSerial& serial_port)
{
    m_Device->ProcessReceivedFrames(m_RxMutex);
    SendPendingCanFrames(serial_port);
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

        std::this_thread::sleep_for(SEND_DELAY_BETWEEN_FRAMES);  /* This delay is needed because UART timeout won't happen if everything is sent at once */
    }
}