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
        SerialRecvFunction recv_function, SerialSendFunction send_function, uint32_t baudrate = 921600) = 0;
    virtual void DeInitInternal() = 0;
    virtual void SetEnabled(bool enable) = 0;
    virtual bool IsEnabled() const = 0;
    virtual void SetComPort(uint16_t port) = 0;
    virtual uint16_t GetComPort() const = 0;
    virtual void SetBaudrate(uint32_t baudrate) = 0;
    virtual uint32_t GetBaudrate() const = 0;
    virtual bool IsOk() const = 0;
};
