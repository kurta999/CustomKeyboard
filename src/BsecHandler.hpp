#pragma once

#include "utils/CSingleton.hpp"
#include <inttypes.h>

#ifdef USE_BSEC
#include "bsec/bsec_datatypes.h"
#else
using bsec_input_t = void;
#endif

class BsecHandler : public CSingleton < BsecHandler >
{
    friend class CSingleton < BsecHandler >;

public:
    BsecHandler() = default;
    ~BsecHandler() = default;

    void Init();
    void AddMeasurementsAndCalculate(int64_t time_stamp, float temp, float pressure, float hum, float gas);
    void SaveCache();

    float GetIaq() const { return iaq; }
    float GetGasPercentage() const { return gas_percentage; }

private:
    int UpdateSubscription(float sample_rate);
    int InitBackend(float sample_rate, float temperature_offset);
    void ProcessData(bsec_input_t* bsec_inputs, uint8_t num_bsec_inputs);

    uint32_t n_samples = 0;

    int64_t timestamp = 0;
    int64_t last_timestamp = 0;
    float iaq = 0.0f;
    uint8_t iaq_accuracy = 0;
    float temp = 0.0f;
    float raw_temp = 0.0f;
    float raw_pressure = 0.0f;
    float humidity = 0.0f;
    float raw_humidity = 0.0f;
    float raw_gas = 0.0f;
    float static_iaq = 0.0f;
    uint8_t static_iaq_accuracy = 0;
    float co2_equivalent = 0.0f;
    uint8_t co2_accuracy = 0;
    float breath_voc_equivalent = 0.0f;
    uint8_t breath_voc_accuracy = 0;
    float comp_gas_value = 0.0f;
    uint8_t comp_gas_accuracy = 0;
    float gas_percentage = 0.0f;
    uint8_t gas_percentage_acccuracy = 0;
};

#define BME680_FLOAT_POINT_COMPENSATION

struct  bme680_field_data {
    /*! Contains new_data, gasm_valid & heat_stab */
    uint8_t status;
    /*! The index of the heater profile used */
    uint8_t gas_index;
    /*! Measurement index to track order */
    uint8_t meas_index;

#ifndef BME680_FLOAT_POINT_COMPENSATION
    /*! Temperature in degree celsius x100 */
    int16_t temperature;
    /*! Pressure in Pascal */
    uint32_t pressure;
    /*! Humidity in % relative humidity x1000 */
    uint32_t humidity;
    /*! Gas resistance in Ohms */
    uint32_t gas_resistance;
#else
    /*! Temperature in degree celsius */
    float temperature;
    /*! Pressure in Pascal */
    float pressure;
    /*! Humidity in % relative humidity x1000 */
    float humidity;
    /*! Gas resistance in Ohms */
    float gas_resistance;

#endif

};
