#pragma once

class CallbackAsyncSerial;
class CanData;

class ICanDevice
{
public:
    virtual ~ICanDevice() { }

    // !\brief Process received CAN frames
    virtual void ProcessReceivedFrames(std::mutex& rx_mutex) = 0;

    // !\brief Send pending CAN frames from message queue
    // !\param serial_port [in] Reference to async serial port
    virtual size_t PrepareSendDataFormat(const std::shared_ptr<CanData>& data_ptr, char* out, size_t max_size, bool& remove_from_queue) = 0;
};