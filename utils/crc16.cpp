#include "crc16.h"

uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    int i;
    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}

uint16_t crc16_calculate(uint8_t* data_in, uint16_t data_length)
{
    uint16_t crc_calc = 0xFFFF;
    for (uint8_t* p = data_in; p < (data_in + data_length); p++)
    {
        crc_calc = crc16_update(crc_calc, *p);
    }
    return crc_calc;
}