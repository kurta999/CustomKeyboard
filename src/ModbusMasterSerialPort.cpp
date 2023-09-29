#include "pch.hpp"

/* THIS MODULE IS IN VERY EARLY DEVELOPMENT STATE, FAR AWAY FROM FINISHED */

constexpr uint32_t RX_QUEUE_MAX_SIZE = 1000;

constexpr auto SERIAL_PORT_TIMEOUT = 1000ms;
constexpr auto SERIAL_PORT_EXCEPTION_TIMEOUT = 1000ms;

ModbusMasterSerialPort::ModbusMasterSerialPort()
{
    
}

ModbusMasterSerialPort::~ModbusMasterSerialPort()
{
    is_notification_pending = true;
    m_RecvCv.notify_all();
}

enum ModbusFunctionCodes : uint8_t
{
    FC_ReadCoilStatus = 1,
    FC_ReadInputStatus = 2,
    FC_ReadHoldingRegister = 3,
    FC_ReadInputRegister = 4,
    FC_ForceSingleCoil = 5,
    FC_WriteSingleRegister = 6,
    FC_WriteMultipleRegister = 16,
};

ModbusMasterSerialPort::ResponseStatus ModbusMasterSerialPort::WaitForResponse()
{
    std::unique_lock lock{ m_RecvMutex };
    m_RecvCv.wait_for(lock, 1000ms, []() { return 0 == 1; });
    return ResponseStatus::Ok;
}

ModbusMasterSerialPort::ResponseStatus ModbusMasterSerialPort::NotifyAndWaitForResponse()
{
    is_notification_pending = true;
    m_cv.notify_all();

    std::unique_lock lock{ m_RecvMutex };
    bool cv_ret = m_RecvCv.wait_for(lock, 3000ms, [this]() { return m_RecvData.size() > 0; });

    ResponseStatus ret = ResponseStatus::Ok;
    if(cv_ret == 0)
        ret = ResponseStatus::Timeout;
    else if(!m_LastDataCrcOk)
        ret = ResponseStatus::CrcError;
    return ret;
}

std::vector<uint8_t> ModbusMasterSerialPort::ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadCoilStatus) };
    vec.push_back(read_offset >> 8 & 0xFF);
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);
    m_SentData = vec;
    
    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse();
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
    vec.push_back(write_offset >> 8 & 0xFF);
    vec.push_back(write_offset & 0xFF);
    
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

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);

    m_SentData = vec;
    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse();
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

std::vector<uint8_t> ModbusMasterSerialPort::ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec = { slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputStatus) };
    vec.push_back(read_offset >> 8 & 0xFF);
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);

    m_SentData = vec;
    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse();
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
    vec.push_back(read_offset >> 8 & 0xFF);    
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);

    m_SentData = vec;
    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse();
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

    m_SentData = vec;
    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse();
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
    vec.push_back(read_offset >> 8 & 0xFF);    
    vec.push_back(read_offset & 0xFF);
    vec.push_back(read_count >> 8 & 0xFF);
    vec.push_back(read_count & 0xFF);

    uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
    vec.push_back(crc & 0xFF);
    vec.push_back(crc >> 8 & 0xFF);

    m_SentData = vec;
    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse();
    if(response == ResponseStatus::Ok)
    {
        uint8_t num_bytes = m_RecvData[2];
        for(int i = 3; i != m_RecvData.size(); i += 2)
        {
            ret.push_back(m_RecvData[i + 1] & 0xFF | m_RecvData[i] << 8);
        }
    }
    m_RecvData.clear();
    return ret;
}

void ModbusMasterSerialPort::Init()
{
    if(is_enabled)
    {
        auto recv_f = std::bind(&ModbusMasterSerialPort::OnUartDataReceived, this, std::placeholders::_1, std::placeholders::_2);
        auto send_f = std::bind(&ModbusMasterSerialPort::OnDataSent, this, std::placeholders::_1);
        InitInternal("ModbusMasterSerialPort", SERIAL_PORT_TIMEOUT, SERIAL_PORT_EXCEPTION_TIMEOUT, recv_f, send_f);
    }
    else
    {
        if(m_worker)
            m_worker->request_stop();
    }
}

void ModbusMasterSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    std::scoped_lock lock{ m_RecvMutex };

    m_RecvData.assign(data, data + (len - 2));

    uint16_t crc = utils::crc16_modbus((void*)data, len - 2);
    if(crc == (uint16_t)(data[len - 1] << 8 | data[len - 2] & 0xFF))
    {
        m_LastDataCrcOk = true;

        std::string hex;
        utils::ConvertHexBufferToString(data, len, hex);
        LOG(LogLevel::Verbose, "Modbus data OK: {}", hex);
    }
    else
    {
        m_LastDataCrcOk = false;

        std::string hex;
        utils::ConvertHexBufferToString(data, len, hex);
        LOG(LogLevel::Verbose, "Modbus data ERR CRC: {}", hex);
    }
    m_RecvCv.notify_all();
}

void ModbusMasterSerialPort::OnDataSent(CallbackAsyncSerial& serial_port)
{
    if(!m_SentData.empty())
    {
        serial_port.write((const char*)m_SentData.data(), m_SentData.size());
        m_SentData.clear();
    }
}