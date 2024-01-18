#include "pch.hpp"

constexpr uint32_t RX_QUEUE_MAX_SIZE = 1000;

constexpr auto SERIAL_PORT_TIMEOUT = 100ms;
constexpr auto SERIAL_PORT_EXCEPTION_TIMEOUT = 1000ms;
constexpr auto RESPONSE_TIMEOUT = 5000ms;

template <typename In> inline void WriteToByteBuffer(std::vector<uint8_t>& vec, const In& data)
{
    using T = std::decay_t<decltype(data)>;
    if constexpr(is_any<T, uint8_t, int8_t>)
    {
        vec.push_back(data);
    }
    else if constexpr(is_any<T, uint16_t, int16_t>)
    {
        vec.push_back(data >> 8 & 0xFF);
        vec.push_back(data & 0xFF);
    }
    else
        static_assert(always_false_v<T>, "bad type - WriteToByteBuffer!");
}

ModbusMasterSerialPort::ModbusMasterSerialPort()
{
    
}

ModbusMasterSerialPort::~ModbusMasterSerialPort()
{
    std::unique_lock lock{ m_RecvMutex };
    m_RecvData.push_back(10);
    m_RecvCv.notify_all();
}

bool ModbusMasterSerialPort::WaitForResponse()
{
    std::unique_lock lock{ m_RecvMutex };
    bool ret = m_RecvCv.wait_for(lock, *m_stopToken, std::chrono::milliseconds(m_ResponseTimeout), [this]() { return m_RecvData.size() > 0; });
    return ret;
}

ModbusMasterSerialPort::ResponseStatus ModbusMasterSerialPort::NotifyAndWaitForResponse(const std::vector<uint8_t>& vec)
{
    m_SentData = vec;
    is_notification_pending = true;

    bool cv_ret = WaitForResponse();

    ResponseStatus ret = ResponseStatus::Timeout;
    if(cv_ret)
    {
        if(!m_LastDataCrcOk)
        {
            ret = ResponseStatus::CrcError;
        }
        else
        {
            if(m_RecvData.size() == 3) /* Probably exception */
            {
                uint16_t func_code = m_RecvData[2];
                if((func_code >= 129 && func_code <= 134) || (func_code >= 143 && func_code <= 144))
                {
                    modbusErrorCount[func_code]++;
                    ret = ResponseStatus::ModbusError;
                    return ret;
                }
            }

            ret = ResponseStatus::Ok;
        }
    }
    return ret;
}

void ModbusMasterSerialPort::AddCrcToFrame(std::vector<uint8_t>& vec)
{
    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);
}

std::vector<uint8_t> ModbusMasterSerialPort::ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadCoilStatus) };
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);
    
    std::vector<uint8_t> ret;
    LOG(LogLevel::Verbose, "NotifyAndWaitForResponse");
    auto response = NotifyAndWaitForResponse(vec);
    LOG(LogLevel::Verbose, "response");
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    return ret;
}

std::vector<uint8_t> ModbusMasterSerialPort::ForceSingleCoil(uint8_t slave_id, uint16_t write_offset, bool status)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ForceSingleCoil) };
    WriteToByteBuffer(vec, write_offset);
    
    if(status)
    {
        vec.push_back(0xFF);
        vec.push_back(0x0);
    }
    else
    {
        vec.push_back(0x0);
        vec.push_back(0x0);
    }
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
        else
        {
            if(m_RecvData.size() == 3)
            {

            }
        }
    }

    m_RecvData.clear();
    return vec;
}

std::vector<uint8_t> ModbusMasterSerialPort::ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputStatus) };
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    return ret;
}

std::vector<uint16_t> ModbusMasterSerialPort::ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadHoldingRegister) };
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            for(int i = 3; i != m_RecvData.size(); i += 2)
            {
                ret.push_back(m_RecvData[i + 1] & 0xFF | m_RecvData[i] << 8);
            }
        }
    }
    m_RecvData.clear();
    return ret;
}

std::vector<uint8_t> ModbusMasterSerialPort::WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer)
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
        WriteToByteBuffer(vec, write_offset);
        WriteToByteBuffer(vec, write_count);
        vec.push_back(buffer.size() * 2);
        for(auto& i : buffer)
        {
            vec.push_back(i >> 8 & 0xFF);
            vec.push_back(i & 0xFF);
        }
    }
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    return vec;
}

std::vector<uint16_t> ModbusMasterSerialPort::ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputRegister) };
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    if(response == ResponseStatus::Ok)
    {
        uint8_t num_bytes = m_RecvData[2];
        if(m_RecvData.size() > 3)
        {
            for(int i = 3; i != m_RecvData.size(); i += 2)
            {
                ret.push_back(m_RecvData[i + 1] & 0xFF | m_RecvData[i] << 8);
            }
        }
    }
    m_RecvData.clear();
    return ret;
}

void ModbusMasterSerialPort::Init()
{
    if(is_enabled)
    {
        auto recv_f = [this](const char* data, unsigned int len) -> void { OnUartDataReceived(data, len); };
        auto send_f = [this](CallbackAsyncSerial& serial_port) -> void { OnDataSent(serial_port); };
        InitInternal("ModbusMasterSerialPort", SERIAL_PORT_TIMEOUT, SERIAL_PORT_EXCEPTION_TIMEOUT, recv_f, send_f);
    }
    else
    {
        m_worker.reset();
    }
}

void ModbusMasterSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    LOG(LogLevel::Verbose, "OnUartDataReceived");
    //std::scoped_lock lock{ m_RecvMutex };

    m_RecvData.assign(data, data + (len - 2));

    uint16_t crc = utils::crc16_modbus((void*)data, len - 2);
    if(crc == (uint16_t)(data[len - 1] << 8 | data[len - 2] & 0xFF))
    {
        m_LastDataCrcOk = true;
    }
    else
    {
        m_LastDataCrcOk = false;
    }

    if(m_Helper)
    {
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        
        std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
        if(modbus_handler && modbus_handler->is_recoding)
        {
            std::scoped_lock lock{ modbus_handler->m };
            modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(CAN_LOG_DIR_RX, static_cast<uint8_t>(data[1]), (uint8_t*)data, static_cast<size_t>(len), t1));
        }
    }
    m_RecvCv.notify_all();
}

void ModbusMasterSerialPort::OnDataSent(CallbackAsyncSerial& serial_port)
{
    if(!m_SentData.empty())
    {
        size_t data_size = m_SentData.size();

        if(m_Helper)
        {
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
            
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            if(modbus_handler && modbus_handler->is_recoding)
            {
                std::scoped_lock lock{ modbus_handler->m };
                modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(CAN_LOG_DIR_TX, static_cast<uint8_t>(m_SentData[1]), m_SentData.data(), data_size, t1));
            }
        }

        serial_port.write((const char*)m_SentData.data(), data_size);
        m_SentData.clear();
    }
}