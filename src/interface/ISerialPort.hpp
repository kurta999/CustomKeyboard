#pragma once

#include <string>

class CallbackAsyncSerial;

using SerialRecvFunction = std::function<void(const char*, size_t)>;
using SerialSendFunction = std::function<void(CallbackAsyncSerial&)>;

class ISerialPort
{
public:
    virtual ~ISerialPort() = default;

    virtual void InitInternal(const std::string& serial_name, std::chrono::milliseconds main_timeout, std::chrono::milliseconds exception_timeout,
        SerialRecvFunction recv_function, SerialSendFunction send_function, uint32_t baudrate = 921600, bool auto_open = false) = 0;
    virtual void InitInternal(const std::string& ip, uint16_t port, bool auto_open, std::chrono::milliseconds main_timeout, std::chrono::milliseconds exception_timeout,
        SerialRecvFunction recv_function, SerialSendFunction send_function) = 0;
    virtual void DeInitInternal() = 0;
    virtual void SetEnabled(bool enable) = 0;
    virtual bool IsEnabled() const = 0;
    virtual void SetTcp(bool is_tcp) = 0;
    virtual bool IsTcp() const = 0;

    virtual void SetTcpIp(const std::string& ip) = 0;
    virtual const std::string& GetTcpIp() const = 0;
    virtual void SetTcpPort(uint16_t port) = 0;
    virtual uint16_t GetTcpPort() const = 0;

    virtual bool IsOpen() = 0;
    virtual void Open() = 0;
    virtual void Close() = 0;

    virtual void SetComPort(uint16_t port) = 0;
    virtual uint16_t GetComPort() const = 0;
    virtual void SetBaudrate(uint32_t baudrate) = 0;
    virtual uint32_t GetBaudrate() const = 0;
    virtual bool IsOk() const = 0;
    virtual bool IsErrorPresent() const = 0;
};
