#pragma once

#include <inttypes.h>
#include <ICanDevice.hpp>

class CanDeviceLawicel : public ICanDevice
{
public:
    CanDeviceLawicel(boost::circular_buffer<char>& CircBuff);
    ~CanDeviceLawicel();

    void ProcessReceivedFrames(std::mutex& rx_mutex) override;
    size_t PrepareSendDataFormat(const std::shared_ptr<CanData>& data_ptr, char* out, size_t size, bool& remove_from_queue) override;

private:
    boost::circular_buffer<char>& m_CircBuff;
    uint8_t device_state = 0;
};