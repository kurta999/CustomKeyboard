#pragma once


#include "ISerialPort.hpp"
#include <inttypes.h>
#include <thread>

#include "utils/AsyncSerial.hpp"

class SerialPortBase : public ISerialPort
{
public:
    SerialPortBase() = default;
    ~SerialPortBase();

    void InitInternal(const std::string& serial_name, std::chrono::milliseconds main_timeout, std::chrono::milliseconds exception_timeout,
        SerialRecvFunction recv_function, SerialSendFunction send_function, uint32_t baudrate = 921600, bool auto_open = true) override;
    void InitInternal(const std::string& ip, uint16_t port, bool auto_open, std::chrono::milliseconds main_timeout, std::chrono::milliseconds exception_timeout,
        SerialRecvFunction recv_function, SerialSendFunction send_function) override;
    void DeInitInternal() override;
    void SetEnabled(bool enable) override;
    bool IsEnabled() const override;

    void SetTcp(bool is_tcp_) override;
    bool IsTcp() const override;

    void SetTcpIp(const std::string& ip) override;
    const std::string& GetTcpIp() const override;
    void SetTcpPort(uint16_t port) override;
    uint16_t GetTcpPort() const override;

    bool IsOpen() override;
    void Open() override;
    void Close() override;

    void SetComPort(uint16_t port) override;
    uint16_t GetComPort() const override;
    virtual void SetBaudrate(uint32_t baudrate) override;
    virtual uint32_t GetBaudrate() const override;
    bool IsOk() const override;
    bool IsErrorPresent() const override;
    void NotifiyMainThread();
    void DestroyWorkerThread();
    bool IsInstanceInited();

protected:
    std::unique_ptr<CallbackAsyncSerial> m_serial;

    std::string m_SerialName;

    SerialRecvFunction m_RecvFunction = {};

    SerialSendFunction m_SendFunction = {};

    std::chrono::milliseconds m_MainTimeout;
    
    std::chrono::milliseconds m_ExceptionTimeout;

    void WorkerThread(std::stop_token token);

    // !\brief Is serial port data receiving enabled?
    bool is_enabled = true;

    bool is_tcp = false;

    bool m_IsAutoOpen = false;
    
    std::string m_TcpIp;

    uint16_t m_TcpPort = 0;

    // !\brief COM port number
    uint16_t com_port = 5;

    // !\brief COM port baudrate
    uint32_t m_Baudrate = 921600;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    // !\brief Is notification pending?
    std::atomic<bool> is_notification_pending = false;

    // !\brief Mutex for main thread
    std::mutex m_mutex;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;

    // !\brief Is serial status OK or ERROR?
    bool m_is_ok = false;
};
