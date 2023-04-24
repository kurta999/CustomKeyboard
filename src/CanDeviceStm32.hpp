#pragma once

#include <inttypes.h>
#include <ICanDevice.hpp>

constexpr uint32_t MAGIC_NUMBER_SEND_DATA_TO_CAN_BUS = 0xAABBCCDD;
constexpr uint32_t MAGIC_NUMBER_RECV_DATA_FROM_CAN_BUS = 0xAABBCCDE;

class CanDeviceStm32 : public ICanDevice
{
public:
    CanDeviceStm32(boost::circular_buffer<char>& CircBuff);
    ~CanDeviceStm32();

    void ProcessReceivedFrames(std::mutex& rx_mutex) override;
    size_t PrepareSendDataFormat(const std::shared_ptr<CanData>& data_ptr, char* out, size_t size, bool& remove_from_queue) override;

private:
    boost::circular_buffer<char>& m_CircBuff;
};