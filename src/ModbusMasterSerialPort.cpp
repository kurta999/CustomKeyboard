#include "pch.hpp"

/* THIS MODULE IS IN VERY EARLY DEVELOPMENT STATE, FAR AWAY FROM FINISHED */

constexpr uint32_t MAGIC_NUMBER_RECV_DATA_ERR = 0xAABBCCDF;

constexpr uint32_t RX_QUEUE_MAX_SIZE = 1000;

ModbusMasterSerialPort::~ModbusMasterSerialPort()
{

}

enum ModbusFunctionCodes : uint8_t
{
    FC_ReadHoldingRegister = 3,
    FC_ReadInputRegister = 3,
    FC_WriteSingleRegister = 6,
    FC_WriteMultipleRegister = 16,
};

std::vector<uint8_t> ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadHoldingRegister) };
    vec.push_back(read_offset >> 8 & 0xFF);    
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);
    return vec;
}

std::vector<uint8_t> WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer)
{
    std::vector<uint8_t> vec;
    if(write_count == 1)  /* Use write single register if only 1 register is written */
    {
        vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_WriteSingleRegister) };
        vec.push_back(write_offset >> 8 & 0xFF);
        vec.push_back(write_offset & 0xFF);
        vec.push_back(buffer[0] >> 8 & 0xFF);
        vec.push_back(buffer[0] & 0xFF);
    }
    else
    {
        vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_WriteMultipleRegister) };
        vec.push_back(write_offset >> 8 & 0xFF);
        vec.push_back(write_offset & 0xFF);
        vec.push_back(write_count >> 8 & 0xFF);
        vec.push_back(write_count & 0xFF);
        vec.push_back(buffer.size() * 2);
        for(auto& i : buffer)
        {
            vec.push_back(i >> 8 & 0xFF);
            vec.push_back(i & 0xFF);
        }
    }
    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);
    return vec;
}

std::vector<uint8_t> ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputRegister) };
    vec.push_back(read_offset >> 8 & 0xFF);    
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);
    return vec;
}

void ModbusMasterSerialPort::Init()
{
    char data[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0A }; // C5 CD 
    uint16_t crc = utils::crc16_modbus(data, sizeof(data));
    
    if(is_enabled)
    {
        if(!m_worker)
            m_worker = std::make_unique<std::jthread>(std::bind(&ModbusMasterSerialPort::UartReceiveThread, this, std::placeholders::_1, std::ref(m_cv), std::ref(m_mutex)));
        if(m_worker)
            utils::SetThreadName(*m_worker, "ModbusMasterSerialPort");
    }
    else
    {
        if(m_worker)
            m_worker->request_stop();
    }
}

void ModbusMasterSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    uint16_t crc = utils::crc16_modbus((void*)data, len - 2);
    if(crc == (uint16_t)(data[len - 1] << 8 | data[len - 2] & 0xFF))
    {
        if(waiting_for_response)
        {
            DBG("data recv");
            waiting_for_response = false;
        }
    }

    
    /*
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
    }*/
}

void ModbusMasterSerialPort::UartReceiveThread(std::stop_token stop_token, std::condition_variable_any& cv, std::mutex& m)
{
    while(!stop_token.stop_requested())
    {
        try
        {
#ifdef _WIN32
            CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 115200);
#else
            CallbackAsyncSerial serial("/dev/ttyUSB" + std::to_string(com_port), 115200);
#endif
            serial.setCallback(std::bind(&ModbusMasterSerialPort::OnUartDataReceived, this, std::placeholders::_1, std::placeholders::_2));

            while(!stop_token.stop_requested())
            {
                if(serial.errorStatus() || serial.isOpen() == false)
                {
                    LOG(LogLevel::Error, "Serial port can unexpectedly closed");
                    break;
                }

                //std::vector<uint8_t> reg = ReadHoldingRegister(0x1, 0, 0xA);
                std::vector<uint8_t> reg = WriteHoldingRegister(0x1, 0, 4, { 35678, 35, 40, 694 });

                serial.write((const char*)reg.data(), reg.size());

                //uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - i->last_execution).count();
                //if(elapsed > i->period)
                {
                    last_tx_time = std::chrono::steady_clock::now();
                    waiting_for_response = true;
                }

                /*
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

                    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                    can_handler->OnFrameSent(d.frame_id, d.data_len, d.data);
                    m_TxQueue.pop_front();
                }
                */
                std::unique_lock lock(m_mutex);
                std::stop_callback stop_wait{ stop_token, [&cv]() { cv.notify_one(); } };
                cv.wait_for(lock, 500ms, [&stop_token]() { return stop_token.stop_requested(); });
            }
            try
            {
                serial.close();
            }
            catch(...)
            {

            }
        }
        catch(const std::exception& e)
        {
            LOG(LogLevel::Error, "Exception MODBUS Master serial: {}", e.what());
            {
                std::unique_lock lock(m_mutex);
                std::stop_callback stop_wait{ stop_token, [&cv]() { cv.notify_one(); } };
                cv.wait_for(lock, 1000ms, [&stop_token]() { return stop_token.stop_requested(); });
            }
        }
    }
}

void ModbusMasterSerialPort::SetEnabled(bool enable)
{
    is_enabled = enable;
}

bool ModbusMasterSerialPort::IsEnabled()
{
    return is_enabled;
}

void ModbusMasterSerialPort::SetComPort(uint16_t port)
{
    com_port = port;
}

uint16_t ModbusMasterSerialPort::GetComPort()
{
    return com_port;
}

void ModbusMasterSerialPort::AddToTxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
    if(!data || !data_len)
        return;
    //m_TxQueue.push_back(std::make_unique<CanData>(frame_id, data_len, data));
}

void ModbusMasterSerialPort::AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data)
{
#if 0
    // Update gui
    m_RxQueue.push_back(std::make_unique<CanData>(frame_id, data_len, data));
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    can_handler->OnFrameReceived(frame_id, data_len, data);

    if(m_RxQueue.size() > RX_QUEUE_MAX_SIZE)  /* TODO: once logging is added, move this to settings.ini */
        m_RxQueue.pop_back();
#endif
}