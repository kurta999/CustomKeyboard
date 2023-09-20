#pragma once

#include "ISerialPort.hpp"
#include <inttypes.h>
#include <thread>

class SerialPortBase : public ISerialPort
{
public:
    SerialPortBase() = default;
    ~SerialPortBase();

    void InitInternal(const std::string& serial_name, SerialRecvFunction recv_function, SerialSendFunction send_function) override;
    void SetEnabled(bool enable) override;
    bool IsEnabled() const override;
    void SetComPort(uint16_t port) override;
    uint16_t GetComPort() const override;
    void NotifiyMainThread();
    void DestroyWorkerThread();

private:
    std::string m_SerialName;

    SerialRecvFunction m_RecvFunction = {};

    SerialSendFunction m_SendFunction = {};


    void WorkerThread(std::stop_token token);

    // !\brief Is serial port data receiving enabled?
    bool is_enabled = true;

    // !\brief COM port number
    uint16_t com_port = 5;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    // !\brief Is notification pending?
    std::atomic<bool> is_notification_pending = false;

    // !\brief Mutex for main thread
    std::mutex m_mutex;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;
};
