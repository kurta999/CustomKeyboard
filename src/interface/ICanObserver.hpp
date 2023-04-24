#pragma once

#include <inttypes.h>

class ICanObserver
{
public:
    virtual ~ICanObserver() {};

    virtual void OnFrameOnBus(uint32_t frame_id, uint8_t* data, uint16_t size) = 0;
    virtual void OnIsoTpDataReceived(uint32_t frame_id, uint8_t* data, uint16_t size) = 0;
};
