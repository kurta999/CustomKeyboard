#include "pch.hpp"

constexpr uint32_t RX_QUEUE_MAX_SIZE = 1000;

constexpr auto SERIAL_PORT_TIMEOUT = 100ms;
constexpr auto SERIAL_PORT_EXCEPTION_TIMEOUT = 1000ms;
constexpr auto RESPONSE_TIMEOUT = 5000ms;

constexpr size_t MAX_HOLDING_REG_ONCE = 120;

//std::binary_semaphore m_RxSem{ 0 };

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
    bool ret = m_RecvCv.wait_for(lock, *m_stopToken, std::chrono::milliseconds(m_ResponseTimeout), [this]()
        {
            return m_RecvData.size() > 0;
        });
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
        if(IsTcp())
            m_LastDataCrcOk = true;

        if(m_Helper)
        {
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            if(modbus_handler && modbus_handler->is_recoding)
            {
                std::scoped_lock lock{ modbus_handler->m };
                ModbusErorrType error_type = m_LastDataCrcOk ? ModbusErorrType::MB_ERR_OK : ModbusErorrType::MB_ERR_CRC;
                uint8_t fc = 0;
                if(!IsTcp() && m_RecvData.size() > 1)
                    fc = m_RecvData[1];
                else if(IsTcp() && m_RecvData.size() > 7)
                    fc = m_RecvData[7];
                size_t len = !IsTcp() ? m_RecvData.size() : m_RecvData.size() - 6;

                modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(MODBUS_LOG_DIR_RX, fc, error_type,
                    (uint8_t*)m_RecvData.data(), len, t1));
            }
        }

        if(!m_LastDataCrcOk)
        {
            ret = ResponseStatus::CrcError;
        }
        else
        {
            if((!IsTcp() && m_RecvData.size() == 3) || (IsTcp() && m_RecvData.size() == 9)) /* Probably exception */
            {
                uint16_t err_response = !IsTcp() ? m_RecvData[1] : m_RecvData[8];
                if((err_response >= 129 && err_response <= 134) || (err_response >= 143 && err_response <= 144))
                {
                    modbusErrorCount[err_response]++;
                    ret = ResponseStatus::ModbusError;

                    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
                    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                    //if(!gIsAppInited) return ret;

                    if(modbus_handler && modbus_handler->is_recoding)
                    {
                        uint8_t fc = 0xFF;
                        if(!IsTcp() && m_RecvData.size() > 1)
                            fc = m_RecvData[1];
                        else if(IsTcp() && m_RecvData.size() > 7)
                            fc = m_RecvData[7];
                        size_t len = !IsTcp() ? m_RecvData.size() : m_RecvData.size() - 6;

                        modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(MODBUS_LOG_DIR_TX, fc,
                            (ModbusErorrType)err_response, m_RecvData.data(), len, t1));
                    }
                    return ret;
                }
            }

            ret = ResponseStatus::Ok;
        }
    }
    else
    {
        //if(!gIsAppInited) return ret;

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
        if(modbus_handler && modbus_handler->is_recoding)
        {
            uint8_t fc = m_SentData.size() > 2 ? static_cast<uint8_t>(m_SentData[1]) : 0xFF;
            modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(MODBUS_LOG_DIR_RX, fc,
                ModbusErorrType::MB_ERR_TIMEOUT, m_SentData.data(), m_SentData.size(), t1));
        }
    }
    return ret;
}

void ModbusMasterSerialPort::AddCrcToFrame(std::vector<uint8_t>& vec)
{
    if(!IsTcp())
    {
        uint16_t crc = utils::crc16_modbus(vec.data(), vec.size());
        vec.push_back(crc & 0xFF);
        vec.push_back(crc >> 8 & 0xFF);
    }
    else
    {

    }
}

void ModbusMasterSerialPort::SetupHeader(std::vector<uint8_t>& vec, uint8_t slave_id, uint16_t fcode, uint16_t len)
{
    if(IsTcp())
    {
        WriteToByteBuffer<uint16_t>(vec, sequence_id);  /* Transaction ID*/
        WriteToByteBuffer<uint16_t>(vec, 0x0);  /* Protocol ID - always 00*/
        WriteToByteBuffer<uint16_t>(vec, len + 2);  /* Length*/
        WriteToByteBuffer<uint8_t>(vec, slave_id);
        WriteToByteBuffer<uint8_t>(vec, fcode);

        sequence_id++;
    }
    else
    {
        WriteToByteBuffer<uint16_t>(vec, slave_id);
        WriteToByteBuffer<uint16_t>(vec, fcode);
    }
}

void ModbusMasterSerialPort::DoCleanup(std::vector<uint8_t>& recv_data)
{
    if(IsTcp())
    {
        if(recv_data.size() > 3)
        {
            recv_data.erase(recv_data.begin(), recv_data.begin() + 3);  /* Erase Transaction ID & Protocol ID*/
            recv_data.erase(recv_data.begin() + 2, recv_data.begin() + 5);  /* Erase Transaction ID & Protocol ID*/
        }
    }
}

std::expected<std::vector<uint8_t>, ModbusError> ModbusMasterSerialPort::ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec;
    SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadCoilStatus), 4);
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    //m_RxSem.acquire();
    DoCleanup(m_RecvData);
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            std::copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    //m_RxSem.release();
    return ret;
}

std::expected<std::vector<uint8_t>, ModbusError> ModbusMasterSerialPort::ForceSingleCoil(uint8_t slave_id, uint16_t write_offset, bool status)
{
    std::vector<uint8_t> vec;
    SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ForceSingleCoil), 6);
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
    DoCleanup(m_RecvData);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    //m_RxSem.acquire();
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            std::copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
        else
        {
            if(m_RecvData.size() == 3)
            {

            }
        }
    }

    m_RecvData.clear();
    //m_RxSem.release();
    return vec;
}

std::expected<std::vector<uint8_t>, ModbusError> ModbusMasterSerialPort::ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec;
    SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputStatus), 4);
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    DoCleanup(m_RecvData);
    //m_RxSem.acquire();
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            std::copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    //m_RxSem.release();
    return ret;
}

std::expected<std::vector<uint16_t>, ModbusError> ModbusMasterSerialPort::ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec;
    SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadHoldingRegister), 4);
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    DoCleanup(m_RecvData);
    //m_RxSem.acquire();
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            for(int i = 3; i != m_RecvData.size(); i += 2)
            {
                if(i + 1 >= m_RecvData.size())
                    break;
                uint16_t reg = m_RecvData[i + 1] & 0xFF | m_RecvData[i] << 8;
                ret.push_back(reg);
            }
        }
    }
    m_RecvData.clear();
    //m_RxSem.release();
    return ret;
}

std::expected<std::vector<uint16_t>, ModbusError> ModbusMasterSerialPort::ReadHoldingRegisters(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint16_t> result;
    size_t remaining = read_count;
    uint16_t offset = read_offset;
    while(remaining > 0)
    {
        size_t step = std::min<uint16_t>(remaining, MAX_HOLDING_REG_ONCE);

        auto tmp = ReadHoldingRegister(slave_id, offset, step);
        if(tmp.has_value() && !tmp->empty())
        {
            offset += step;
            remaining -= step;

            result.insert(result.end(), tmp->begin(), tmp->end());
        }
        else
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    //m_RxSem.release();
    return result;
}

std::expected<std::vector<uint8_t>, ModbusError> ModbusMasterSerialPort::WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer)
{
    std::vector<uint8_t> vec;
    if(write_count == 1)  /* Use write single register if only 1 register is written */
    {
        SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_WriteSingleRegister), 4);
        vec.push_back(write_offset >> 8 & 0xFF);
        vec.push_back(write_offset & 0xFF);
        vec.push_back(buffer[0] >> 8 & 0xFF);
        vec.push_back(buffer[0] & 0xFF);
    }
    else
    {
        size_t buffer_size = buffer.size() * 2;
        SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_WriteMultipleRegister), 4 + buffer_size);
        WriteToByteBuffer(vec, write_offset);
        WriteToByteBuffer(vec, write_count);
        vec.push_back(buffer_size);
        for(auto& i : buffer)
        {
            vec.push_back(i >> 8 & 0xFF);
            vec.push_back(i & 0xFF);
        }
    }
    AddCrcToFrame(vec);

    std::vector<uint8_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    DoCleanup(m_RecvData);
    //m_RxSem.acquire();
    if(response == ResponseStatus::Ok)
    {
        if(m_RecvData.size() > 3)
        {
            uint8_t num_bytes = m_RecvData[2];
            std::copy(m_RecvData.begin() + 3, m_RecvData.end(), back_inserter(ret));
        }
    }
    m_RecvData.clear();
    //m_RxSem.release();
    return vec;
}

std::expected<std::vector<uint16_t>, ModbusError> ModbusMasterSerialPort::ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count)
{
    std::vector<uint8_t> vec;
    SetupHeader(vec, slave_id, static_cast<uint8_t>(ModbusFunctionCodes::FC_ReadInputRegister), 4);
    WriteToByteBuffer(vec, read_offset);
    WriteToByteBuffer(vec, read_count);
    AddCrcToFrame(vec);

    std::vector<uint16_t> ret;
    auto response = NotifyAndWaitForResponse(vec);
    DoCleanup(m_RecvData);
    //m_RxSem.acquire();
    if(response == ResponseStatus::Ok)
    {
        uint8_t num_bytes = m_RecvData[2];
        if(m_RecvData.size() > 3)
        {
            for(int i = 3; i != m_RecvData.size(); i += 2)
            {
                if(i + 1 >= m_RecvData.size())
                    break;

                ret.push_back(m_RecvData[i + 1] & 0xFF | m_RecvData[i] << 8);
            }
        }
    }
    m_RecvData.clear();
    //m_RxSem.release();
    return ret;
}

void ModbusMasterSerialPort::Init()
{
    if(is_enabled)
    {
        auto recv_f = [this](const char* data, unsigned int len) -> void
            {
                OnUartDataReceived(data, len);
            };
        auto send_f = [this](CallbackAsyncSerial& serial_port) -> void
            {
                OnDataSent(serial_port);
            };
        InitInternal("ModbusMasterSerialPort", SERIAL_PORT_TIMEOUT, SERIAL_PORT_EXCEPTION_TIMEOUT, recv_f, send_f, 0, false);
    }
    else
    {
        m_worker.reset();
    }
}

void ModbusMasterSerialPort::OnUartDataReceived(const char* data, unsigned int len)
{
    //LOG(LogLevel::Verbose, "OnUartDataReceived");
    //std::scoped_lock lock{ m_RecvMutex };

    if(/*m_RecvData.empty() && */len > 2)
    {
        if(IsTcp())
            m_RecvData.assign(data, data + len);
        else
            m_RecvData.assign(data, data + (len - 2));
    }
    else
    {
        LOG(LogLevel::Error, "RX buffer isn't empty!");
    }

    if(!m_RecvData.empty())
    {
        if(!IsTcp())
        {
            uint16_t crc = utils::crc16_modbus((void*)data, len - 2);
            if(crc == (uint16_t)(data[len - 1] << 8 | data[len - 2] & 0xFF))
            {
                m_LastDataCrcOk = true;
            }
            else
            {
                m_LastDataCrcOk = false;
            }
        }
        else
        {
            m_LastDataCrcOk = true;
        }
        m_RecvCv.notify_all();
    }
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
                if(modbus_handler->m_LogEntries.size() >= modbus_handler->max_recorded_entries)
                    modbus_handler->m_LogEntries.clear();

                uint8_t fc = 0;
                if(!IsTcp() && m_SentData.size() > 1)
                    fc = m_SentData[1];
                else if(IsTcp() && m_SentData.size() > 7)
                    fc = m_SentData[7];

                size_t len = !IsTcp() ? data_size : data_size - 6;
                modbus_handler->m_LogEntries.emplace_back(std::make_unique<ModbusLogEntry>(MODBUS_LOG_DIR_TX, fc,
                    ModbusErorrType::MB_ERR_OK, m_SentData.data(), len, t1));
            }
        }

        serial_port.write((const char*)m_SentData.data(), data_size);
        m_LastSentData = std::move(m_SentData);
        m_SentData.clear();
    }
}