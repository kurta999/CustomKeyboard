#include "pch.hpp"

using namespace std::chrono_literals;
using crc16_modbus_t = boost::crc_optimal<16, 0x8005, 0xFFFF, 0, true, true>;

constexpr uint32_t MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS = 0xAABBCCDD;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS = 0xAABBCCDE;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_ERR = 0xAABBCCDF;

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

enum UsbCanState : uint8_t
{
    CAN_DATA_SEND,
    CAN_DATA_RECEIVE,
    CAN_DATA_FINISH,
};

UsbCanState usb_state = CAN_DATA_FINISH;

CanSerialPort::~CanSerialPort()
{
    DestroyWorkingThread();
}

void CanSerialPort::Init()
{
    if(is_enabled)
    {
        if(!m_worker)
            m_worker = std::make_unique<std::thread>(&CanSerialPort::UartReceiveThread, this, std::ref(to_exit), std::ref(m_cv), std::ref(m_mutex));
    }
    else
    {
        DestroyWorkingThread();
    }
}

void CanSerialPort::DestroyWorkingThread()
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
    }
}

void CanSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    if(len != sizeof(UartCanData))
        return;
    UartCanData* d = (UartCanData*)data;
    crc16_modbus_t calc_result;
    calc_result.process_bytes((void*)data, len - 2);
    if(d->magic_number == MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS)
    {
        /*
        uint8_t input_buffer[8];
        memcpy(input_buffer, (uint8_t*)(&data[4]), sizeof(input_buffer));
        */
        AddToRxQueue(d->frame_id, d->data_len, d->data);
    }
    else if(d->magic_number == MAGIC_NUMBER_RECV_DATA_ERR)
    {
        if(!m_TxQueue.empty())
            m_TxQueue.pop_front();
        usb_state = CAN_DATA_FINISH;
    }
}

void CanSerialPort::UartReceiveThread(std::atomic<bool>& to_exit, std::condition_variable& cv, std::mutex& m)
{
    while(!to_exit)
    {
        try
        {
            CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 115200); /* baud rate has no meaning here */
            serial.setCallback(std::bind(&CanSerialPort::OnUartDataReceived, this, std::placeholders::_1, std::placeholders::_2));

            while(!to_exit)
            {
                if(serial.errorStatus() || serial.isOpen() == false)
                {
                    LOGMSG(error, "Serial port can unexpectedly closed");
                    break;
                }

                if(!m_TxQueue.empty())
                {
                    std::shared_ptr<CanData> data_ptr = m_TxQueue.front();

                    crc16_modbus_t calc_result;
                    calc_result.process_bytes((void*)&data_ptr->frame_id, sizeof(CanData));
                    uint16_t crc = calc_result.checksum();

                    UartCanData d;
                    d.magic_number = MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS;
                    d.frame_id = data_ptr->frame_id;
                    d.data_len = data_ptr->data_len;
                    memcpy(d.data, data_ptr->data, d.data_len);
                    d.crc = crc;

                    serial.write((const char*)&d, sizeof(UartCanData));
                    m_TxQueue.pop_front();
                }

                {
                    std::unique_lock lock(m_mutex);
                    m_cv.wait_for(lock, 10ms);
                }
            }
            serial.close();
        }
        catch(std::exception& e)
        {
            LOGMSG(error, "Exception can serial {}", e.what());
            {
                std::unique_lock lock(m_mutex);
                m_cv.wait_for(lock, 1000ms);
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

    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    auto it = can_handler->m_rxData.find(frame_id);
    if(it == can_handler->m_rxData.end())
    {
        can_handler->m_rxData[frame_id] = std::make_unique<CanRxData>(data, data_len);
    }
    else
    {
        can_handler->m_rxData[frame_id]->data.assign(data, data + data_len);
    }
    uint32_t elapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(time_now - can_handler->m_rxData[frame_id]->last_execution).count());
    can_handler->m_rxData[frame_id]->period = elapsed;
    can_handler->m_rxData[frame_id]->count++;
    can_handler->m_rxData[frame_id]->last_execution = std::chrono::steady_clock::now();
}