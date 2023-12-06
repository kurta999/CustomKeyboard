#include "pch.hpp"

constexpr auto SERIAL_EXCEPTION_DELAY = 1000ms;

SerialPortBase::~SerialPortBase()
{
    DeInitInternal();
}

void SerialPortBase::InitInternal(const std::string& serial_name, std::chrono::milliseconds main_timeout, std::chrono::milliseconds exception_timeout,
    SerialRecvFunction recv_function, SerialSendFunction send_function, uint32_t baudrate)
{
    m_SerialName = serial_name;
    m_MainTimeout = main_timeout;
    m_ExceptionTimeout = exception_timeout;
    m_RecvFunction = recv_function;
    m_SendFunction = send_function;
    m_Baudrate = baudrate;

    if(!m_worker)
    {
        m_worker = std::make_unique<std::jthread>(std::bind_front(&SerialPortBase::WorkerThread, this));
        utils::SetThreadName(*m_worker, m_SerialName.c_str());
    }
}

void SerialPortBase::DeInitInternal()
{
    DestroyWorkerThread();
}

void SerialPortBase::SetEnabled(bool enable)
{
    is_enabled = enable;
}

bool SerialPortBase::IsEnabled() const
{
    return is_enabled;
}

void SerialPortBase::SetComPort(uint16_t port)
{
    com_port = port;
}

uint16_t SerialPortBase::GetComPort() const
{
    return com_port;
}

bool SerialPortBase::IsOk() const
{
    return m_is_ok;
}

void SerialPortBase::NotifiyMainThread()
{
    is_notification_pending = true;
    m_cv.notify_all();
}

void SerialPortBase::DestroyWorkerThread()
{
    if(m_worker)
    {
        NotifiyMainThread();
        m_worker.reset(nullptr);
    }
}

void SerialPortBase::WorkerThread(std::stop_token token)
{
    while(!token.stop_requested())
    {
        std::string err_msg;
        try
        {
#ifdef _WIN32
            m_serial = std::make_unique<CallbackAsyncSerial>("\\\\.\\COM" + std::to_string(com_port), m_Baudrate);
#else
            m_serial = std::make_unique<CallbackAsyncSerial>("/dev/ttyUSB" + std::to_string(com_port), m_Baudrate);
#endif
            m_serial->setCallback(m_RecvFunction);

            while(!token.stop_requested())
            {
                if(m_serial->errorStatus() || m_serial->isOpen() == false)
                {
                    LOG(LogLevel::Error, "Serial port can unexpectedly closed");
                    m_is_ok = false;
                    break;
                }

                m_is_ok = true;
                {
                    std::unique_lock lock(m_mutex);
                    auto now = std::chrono::system_clock::now();
                    bool ret = m_cv.wait_until(lock, token, now + m_MainTimeout, [this]() { return is_notification_pending != 0; });
                    /*
                    if(!ret)
                    {
                        LOG(LogLevel::Error, "{} - CV timeout", m_SerialName);
                    }
                    */
                }

                if(m_SendFunction)
                    m_SendFunction(*m_serial);
                is_notification_pending = false;
            }
            try
            {
                m_is_ok = false;
                m_serial->close();
            }
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "Exception {} serial close {}", m_SerialName, e.what());
            }
        }
        catch(const std::exception& e)
        {
            LOG(LogLevel::Error, "Exception {} serial {}", m_SerialName, e.what());
            m_is_ok = false;

            std::unique_lock lock(m_mutex);
            m_cv.wait_for(lock, token, m_ExceptionTimeout, []() { return 1 == 0; });
        }
    }

}