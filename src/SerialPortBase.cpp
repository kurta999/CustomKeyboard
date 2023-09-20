#include "pch.hpp"

constexpr auto CAN_SERIAL_PORT_TIMEOUT = 10000ms;
constexpr auto SERIAL_EXCEPTION_DELAY = 1000ms;

SerialPortBase::~SerialPortBase()
{
    DestroyWorkerThread();
}

void SerialPortBase::InitInternal(const std::string& serial_name, SerialRecvFunction recv_function, SerialSendFunction send_function)
{
    m_SerialName = serial_name;
    m_RecvFunction = recv_function;
    m_SendFunction = send_function;

    if(!m_worker)
    {
        m_worker = std::make_unique<std::jthread>(std::bind_front(&SerialPortBase::WorkerThread, this));
        utils::SetThreadName(*m_worker, m_SerialName.c_str());
    }
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
            CallbackAsyncSerial serial("\\\\.\\COM" + std::to_string(com_port), 921600);
#else
            CallbackAsyncSerial serial("/dev/ttyUSB" + std::to_string(com_port), 921600);
#endif
            serial.setCallback(m_RecvFunction);

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
                        LOG(LogLevel::Error, "{} - CV timeout", m_SerialName);
                    }
                }

                if(m_SendFunction)
                    m_SendFunction(serial);
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
        catch(const std::exception& e)
        {
            err_msg = e.what();
        }

        if(!err_msg.empty())
        {
            LOG(LogLevel::Error, "Exception {} serial {}", m_SerialName, err_msg);
            err_msg.clear();

            std::unique_lock lock(m_mutex);
            m_cv.wait_for(lock, token, SERIAL_EXCEPTION_DELAY, []() { return 1 == 0; });
        }
    }

}