#include "pch.hpp"

constexpr int NUM_USED_OUTPUTS = 11;
constexpr int NUM_SAMPLES_BEFORE_SAVE = 100;

/* Global temperature offset to be subtracted */
static float bme680_temperature_offset_g = 0.0f;

constexpr const char* BSEC_CACHE_FILENAME = "bsec.bin";

void BsecHandler::Init()
{
#ifdef USE_BSEC
	InitBackend(BSEC_SAMPLE_RATE_LP, bme680_temperature_offset_g);
#endif
}

int BsecHandler::UpdateSubscription(float sample_rate)
{
#ifdef USE_BSEC
	bsec_sensor_configuration_t requested_virtual_sensors[NUM_USED_OUTPUTS];
	uint8_t n_requested_virtual_sensors = NUM_USED_OUTPUTS;

	bsec_sensor_configuration_t required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
	uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;

	bsec_library_return_t status = BSEC_OK;

	/* note: Virtual sensors as desired to be added here */
	requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
	requested_virtual_sensors[0].sample_rate = sample_rate;
	requested_virtual_sensors[1].sensor_id = BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE;
	requested_virtual_sensors[1].sample_rate = sample_rate;
	requested_virtual_sensors[2].sensor_id = BSEC_OUTPUT_RAW_PRESSURE;
	requested_virtual_sensors[2].sample_rate = sample_rate;
	requested_virtual_sensors[3].sensor_id = BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY;
	requested_virtual_sensors[3].sample_rate = sample_rate;
	requested_virtual_sensors[4].sensor_id = BSEC_OUTPUT_RAW_GAS;
	requested_virtual_sensors[4].sample_rate = sample_rate;
	requested_virtual_sensors[5].sensor_id = BSEC_OUTPUT_RAW_TEMPERATURE;
	requested_virtual_sensors[5].sample_rate = sample_rate;
	requested_virtual_sensors[6].sensor_id = BSEC_OUTPUT_RAW_HUMIDITY;
	requested_virtual_sensors[6].sample_rate = sample_rate;
	requested_virtual_sensors[7].sensor_id = BSEC_OUTPUT_STATIC_IAQ;
	requested_virtual_sensors[7].sample_rate = sample_rate;
	requested_virtual_sensors[8].sensor_id = BSEC_OUTPUT_CO2_EQUIVALENT;
	requested_virtual_sensors[8].sample_rate = sample_rate;
	requested_virtual_sensors[9].sensor_id = BSEC_OUTPUT_BREATH_VOC_EQUIVALENT;
	requested_virtual_sensors[9].sample_rate = sample_rate;
	requested_virtual_sensors[10].sensor_id = BSEC_OUTPUT_GAS_PERCENTAGE;
	requested_virtual_sensors[10].sample_rate = sample_rate;

	/* Call bsec_update_subscription() to enable/disable the requested virtual sensors */
	status = bsec_update_subscription(requested_virtual_sensors, n_requested_virtual_sensors, required_sensor_settings,
		&n_required_sensor_settings);
#else
	uint8_t status = 0;
#endif
	return status;
}

int BsecHandler::InitBackend(float sample_rate, float temperature_offset)
{
#ifdef USE_BSEC
	bsec_library_return_t bsec_status = BSEC_OK;

	uint8_t bsec_state[BSEC_MAX_PROPERTY_BLOB_SIZE] = { 0 };
	uint8_t bsec_config[BSEC_MAX_PROPERTY_BLOB_SIZE] = { 0 };
	uint8_t work_buffer[BSEC_MAX_PROPERTY_BLOB_SIZE] = { 0 };

	/* Initialize BSEC library */
	int ret = bsec_init();
	if(ret != BSEC_OK)
	{
		return ret;
	}

	if(std::filesystem::exists(BSEC_CACHE_FILENAME))
	{
		std::ifstream f(BSEC_CACHE_FILENAME, std::ios::in | std::ios::binary);
		if(f)
		{
			std::string input = { (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>() };
			ret = bsec_set_state((uint8_t*)input.c_str(), input.length(), work_buffer, sizeof(work_buffer));

			LOG(LogLevel::Normal, "Bsec set_state return {}", ret);
		}
	}

	/* Set temperature offset */
	bme680_temperature_offset_g = temperature_offset;

	/* Call to the function which sets the library with subscription information */
	ret = UpdateSubscription(sample_rate);
	if(ret != BSEC_OK)
	{
		return ret;
	}
#else
	uint8_t ret = 0;
#endif
	return ret;
}

void BsecHandler::AddMeasurementsAndCalculate(int64_t time_stamp, float temp, float pressure, float hum, float gas)
{
#ifdef USE_BSEC
	if(time_stamp == last_timestamp)
	{
		return;
	}

	/* Allocate enough memory for up to BSEC_MAX_PHYSICAL_SENSOR physical inputs*/
	bsec_input_t bsec_inputs[BSEC_MAX_PHYSICAL_SENSOR];

	/* Number of inputs to BSEC */
	uint8_t num_bsec_inputs = 0;
	static struct bme680_field_data data;

	/* BSEC sensor settings struct */
	bsec_bme_settings_t sensor_settings;
	bsec_library_return_t bsec_status = BSEC_OK;

	data.temperature = temp;// * 100.f;
	data.humidity = hum; //hum * 1000.f;
	data.pressure = pressure * 100.f;
	data.gas_resistance = gas;
	/* convert the timestamp in nanoseconds before calling bsec_sensor_control() */
	last_timestamp = timestamp;
	time_stamp = time_stamp * 1000000;

	/* Retrieve sensor settings to be used in this time instant by calling bsec_sensor_control */
	bsec_sensor_control(time_stamp, &sensor_settings);

	/* Place presssure sample into input struct */
	bsec_inputs[num_bsec_inputs].sensor_id = BSEC_INPUT_PRESSURE;
	bsec_inputs[num_bsec_inputs].signal = data.pressure;
	bsec_inputs[num_bsec_inputs].time_stamp = time_stamp;
	num_bsec_inputs++;

	/* Place temperature sample into input struct */
	bsec_inputs[num_bsec_inputs].sensor_id = BSEC_INPUT_TEMPERATURE;
	bsec_inputs[num_bsec_inputs].signal = data.temperature;
	bsec_inputs[num_bsec_inputs].time_stamp = time_stamp;
	num_bsec_inputs++;

	if(bme680_temperature_offset_g > 0.0f) {
		bsec_inputs[num_bsec_inputs].sensor_id = BSEC_INPUT_HEATSOURCE;
		bsec_inputs[num_bsec_inputs].signal = bme680_temperature_offset_g;
		bsec_inputs[num_bsec_inputs].time_stamp = time_stamp;
		num_bsec_inputs++;
	}

	/* Place humidity sample into input struct */
	bsec_inputs[num_bsec_inputs].sensor_id = BSEC_INPUT_HUMIDITY;
	bsec_inputs[num_bsec_inputs].signal = data.humidity;
	bsec_inputs[num_bsec_inputs].time_stamp = time_stamp;
	num_bsec_inputs++;

	/* Place gas sample into input struct */
	bsec_inputs[num_bsec_inputs].sensor_id = BSEC_INPUT_GASRESISTOR;
	bsec_inputs[num_bsec_inputs].signal = data.gas_resistance;
	bsec_inputs[num_bsec_inputs].time_stamp = time_stamp;
	num_bsec_inputs++;

	/* Time to invoke BSEC to perform the actual processing */
	ProcessData(bsec_inputs, num_bsec_inputs);

	/* Increment sample counter */
	n_samples++;

	/* Retrieve and store state if the passed save_intvl */
	if(n_samples >= NUM_SAMPLES_BEFORE_SAVE)
	{
		SaveCache();
		n_samples = 0;
	}
#endif
}

void BsecHandler::SaveCache()
{
#ifdef USE_BSEC
	/* Save state variables */
	uint8_t bsec_state[BSEC_MAX_STATE_BLOB_SIZE];
	uint8_t work_buffer[BSEC_MAX_STATE_BLOB_SIZE];
	uint32_t bsec_state_len = 0;

	int bsec_status = bsec_get_state(0, bsec_state, sizeof(bsec_state), work_buffer, sizeof(work_buffer), &bsec_state_len);
	if(bsec_status == BSEC_OK)
	{
		std::ofstream out(std::string(BSEC_CACHE_FILENAME), std::ofstream::binary);
		out.write((const char*)bsec_state, bsec_state_len);
		out.flush();
	}
	LOG(LogLevel::Normal, "Bsec get_state return {}", bsec_status);
#endif
}

void BsecHandler::ProcessData(bsec_input_t* bsec_inputs, uint8_t num_bsec_inputs)
{
#ifdef USE_BSEC
	/* Output buffer set to the maximum virtual sensor outputs supported */
	bsec_output_t bsec_outputs[BSEC_NUMBER_OUTPUTS];
	uint8_t num_bsec_outputs = 0;
	uint8_t index = 0;

	bsec_library_return_t bsec_status = BSEC_OK;

	/* Check if something should be processed by BSEC */
	if(num_bsec_inputs > 0)
	{
		/* Set number of outputs to the size of the allocated buffer */
		/* BSEC_NUMBER_OUTPUTS to be defined */
		num_bsec_outputs = BSEC_NUMBER_OUTPUTS;

		/* Perform processing of the data by BSEC
		   Note:
		   * The number of outputs you get depends on what you asked for during bsec_update_subscription(). This is
			 handled under bme680_bsec_update_subscription() function in this example file.
		   * The number of actual outputs that are returned is written to num_bsec_outputs. */
		bsec_status = bsec_do_steps(bsec_inputs, num_bsec_inputs, bsec_outputs, &num_bsec_outputs);

		/* Iterate through the outputs and extract the relevant ones. */
		for(index = 0; index < num_bsec_outputs; index++)
		{
			switch(bsec_outputs[index].sensor_id)
			{
				case BSEC_OUTPUT_IAQ:
					iaq = bsec_outputs[index].signal;
					iaq_accuracy = bsec_outputs[index].accuracy;
					break;
				case BSEC_OUTPUT_STATIC_IAQ:
					static_iaq = bsec_outputs[index].signal;
					static_iaq_accuracy = bsec_outputs[index].accuracy;
					break;
				case BSEC_OUTPUT_CO2_EQUIVALENT:
					co2_equivalent = bsec_outputs[index].signal;
					co2_accuracy = bsec_outputs[index].accuracy;
					break;
				case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
					breath_voc_equivalent = bsec_outputs[index].signal;
					breath_voc_accuracy = bsec_outputs[index].accuracy;
					break;
				case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
					temp = bsec_outputs[index].signal;
					break;
				case BSEC_OUTPUT_RAW_PRESSURE:
					raw_pressure = bsec_outputs[index].signal;
					break;
				case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
					humidity = bsec_outputs[index].signal;
					break;
				case BSEC_OUTPUT_RAW_GAS:
					raw_gas = bsec_outputs[index].signal;
					break;
				case BSEC_OUTPUT_RAW_TEMPERATURE:
					raw_temp = bsec_outputs[index].signal;
					break;
				case BSEC_OUTPUT_RAW_HUMIDITY:
					raw_humidity = bsec_outputs[index].signal;
					break;
					/*
				case BSEC_OUTPUT_COMPENSATED_GAS:
					comp_gas_value = bsec_outputs[index].signal;
					comp_gas_accuracy = bsec_outputs[index].accuracy;
					break;
					*/
				case BSEC_OUTPUT_GAS_PERCENTAGE:
					gas_percentage = bsec_outputs[index].signal;
					gas_percentage_acccuracy = bsec_outputs[index].accuracy;
					break;
				default:
					continue;
			}

			/* Assume that all the returned timestamps are the same */
			timestamp = bsec_outputs[index].time_stamp;
		}

		if(bsec_status != BSEC_OK)
		{
			LOG(LogLevel::Normal, "IAQ: {:.1f} - {}, CO2EQ: {:.1f}, Gas: {:.1f} - status: {}\n", iaq, iaq_accuracy, co2_equivalent, gas_percentage, (int)bsec_status);
		}
	}
#endif
}
