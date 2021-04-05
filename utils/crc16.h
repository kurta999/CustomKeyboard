#pragma once

#include <inttypes.h>

uint16_t crc16_update(uint16_t crc, uint8_t a);
uint16_t crc16_calculate(uint8_t* data_in, uint16_t data_length);