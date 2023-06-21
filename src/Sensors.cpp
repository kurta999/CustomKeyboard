#include "pch.hpp"
#include <regex>

void Sensors::Init()
{
    std::error_code ec;
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs", ec);
    if(ec)
        LOG(LogLevel::Error, "Error with create_directory (Graphs): {}", ec.message());

    std::ifstream t("Graphs/template.html", std::ifstream::binary);
    if(t)
    {
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        t.seekg(0);
        template_str.resize(size);

        t.read(&template_str[0], size);
        t.close();
    }
    else
    {
        LOG(LogLevel::Error, "Missing template.html from 'Graphs' folder, disabling sensor's related module.");
    }
}

bool Sensors::ProcessIncommingData(const char* recv_data, size_t data_len, const char* from_ip)
{
    bool ret = true;
    try
    {  // MEAS_DATA SCD30: 25.7343,60.9436,717.7289 BME680: 24.0600,62.7400,1001.8600,75693,4253974 HONEYWELL: 3,4 VEML6070: 0 TCS: 114.1704,108.3750,107.7955,4841,92
        std::string s = std::string(recv_data, recv_data + data_len);
        boost::algorithm::erase_all(s, "SCD30");
        boost::algorithm::erase_all(s, "CO");
        boost::algorithm::erase_all(s, "BME680");
        boost::algorithm::erase_all(s, "VEML6070");
        boost::algorithm::erase_all(s, "TCS");

        std::regex num_regex("[-+]?(\\d+([.]\\d*)?|[.]\\d+)([eE][-+]?\\d+)?");
        auto num_begin = std::sregex_iterator(s.begin(), s.end(), num_regex);
        auto num_end = std::sregex_iterator();

        std::vector<std::string> meas_vec;
        for(std::sregex_iterator i = num_begin; i != num_end; ++i) 
        {
            std::smatch match = *i;
            meas_vec.push_back(match.str());
        }

        if(meas_vec.size() < MEAS_INDEX_Max)
        {
            LOG(LogLevel::Warning, "Too few measurements received from {} (data: {}, len: {})", from_ip, recv_data, data_len);
            return false;
        }
        HandleMeasurements(meas_vec);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "std::exception: {} (data: {}, len: {})", e.what(), from_ip, recv_data, data_len);
        return false;
    }
    return true;
}

void Sensors::HandleMeasurements(std::vector<std::string>& meas_vec)
{
    float temp = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_BME680_TEMP]),
        hum = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_BME680_HUM]),
        pressure = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_BME680_PRESSURE]);
    int co2 = utils::stoi<int>(meas_vec[MEAS_INDEX_SCD_CO2]),
        pm25 = utils::stoi<int>(meas_vec[MEAS_INDEX_PM25]), pm10 = utils::stoi<int>(meas_vec[MEAS_INDEX_PM10]), uv = utils::stoi<int>(meas_vec[MEAS_INDEX_UV]);

    int gas_resistance = utils::stoi<int>(meas_vec[MEAS_INDEX_BME680_GAS_RESISTANCE]);
    int b_timestamp = utils::stoi<int>(meas_vec[MEAS_INDEX_BME680_TIMESTAMP]);
    BsecHandler::Get()->AddMeasurementsAndCalculate(b_timestamp, temp, pressure, hum, gas_resistance);
    float voc = BsecHandler::Get()->GetIaq();

    float r = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_R]), g = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_G]), b = boost::lexical_cast<float>(meas_vec[MEAS_INDEX_B]);
    uint16_t lux = utils::stoi<int>(meas_vec[MEAS_INDEX_Lux]), cct = utils::stoi<int>(meas_vec[MEAS_INDEX_CCT]);
    int co = utils::stoi<int>(meas_vec[MEAS_INDEX_CO]);

#ifdef _WIN32 /* TODO: remove once std::chrono::current_zone() support is added to clang */
    const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

    Measurement temp_meas(temp, hum, co2, voc, co, pm25, pm10, pressure, r, g, b, lux, cct, uv, std::move(std::format("{:%H:%M:%OS}", now)));

    bool do_not_update = false;
    if(m_CurrMeas == nullptr)
    {
        //LOG(LogLevel::Notification, "Add new meas");

        m_CurrMeas = std::make_unique<Measurement>(temp_meas);
        m_IntegrationStartTimestamp = std::chrono::steady_clock::now();
        do_not_update = true;
    }

    UpdateGui(temp_meas);
    num_recv_meas++;

    bool add_to_db = false;
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_IntegrationStartTimestamp).count();
    if(time_elapsed > m_IntegrationTime)
    {
        //LOG(LogLevel::Notification, "Add to db");
        m_CurrMeas->Finalize();
        DatabaseLogic::Get()->InsertMeasurement(m_CurrMeas);
        AddMeasurement(std::move(m_CurrMeas));
        m_CurrMeas = nullptr;

        UpdateDatabaseIfNeeded();
    }
    else
    {
        if(!do_not_update)
        {
            *m_CurrMeas += temp_meas;
            //LOG(LogLevel::Notification, "Update {}", m_CurrMeas->cnt);
        }
    }
#endif
}

void Sensors::WriteGraphs()
{
    WriteGraph<decltype(Measurement::temp)>("Temperature.html", 15, 35, "Temperature", offsetof(Measurement, temp));
    WriteGraph<decltype(Measurement::hum)>("Humidity.html", 0, 100, "Humidity", offsetof(Measurement, hum));
    WriteGraph<decltype(Measurement::co2)>("CO2.html", 150, 5000, "CO2", offsetof(Measurement, co2));
    WriteGraph<decltype(Measurement::voc)>("VOC.html", 0, 1000, "VOC", offsetof(Measurement, voc));
    WriteGraph<decltype(Measurement::co)>("CO.html", 0, 65535, "CO", offsetof(Measurement, co));
    WriteGraph<decltype(Measurement::pm25)>("PM25.html", 0, 1000, "PM2.5", offsetof(Measurement, pm25));
    WriteGraph<decltype(Measurement::pm10)>("PM10.html", 0, 1000, "PM10", offsetof(Measurement, pm10));
    WriteGraph<decltype(Measurement::pressure)>("Pressure.html", 950, 1200, "Pressure", offsetof(Measurement, pressure));
    WriteGraph<decltype(Measurement::lux)>("Lux.html", 0, 10000, "Lux", offsetof(Measurement, lux));
    WriteGraph<decltype(Measurement::cct)>("CCT.html", 0, 10000, "CCT", offsetof(Measurement, cct));
    WriteGraph<decltype(Measurement::r)>("R.html", 0, 10000, "R", offsetof(Measurement, r));
    WriteGraph<decltype(Measurement::g)>("G.html", 0, 10000, "G", offsetof(Measurement, g));
    WriteGraph<decltype(Measurement::b)>("B.html", 0, 10000, "B", offsetof(Measurement, b));
    WriteGraph<decltype(Measurement::uv)>("UV.html", 0, 10000, "UV", offsetof(Measurement, uv));
}

void Sensors::AddMeasurement(std::unique_ptr<Measurement>&& meas)
{
    size_t size = last_meas.size();
    if(size >= m_GraphResolution)
        last_meas.pop_front();

    last_meas.push_back(std::move(meas));
}

void Sensors::UpdateGui(Measurement& meas)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(wxGetApp().is_init_finished && frame && frame->main_panel)
    {
        const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        frame->main_panel->m_textTemp->SetLabelText(wxString::Format(wxT("Temperature: %.1f"), meas.temp));
        frame->main_panel->m_textHum->SetLabelText(wxString::Format(wxT("Humidity: %.1f"), meas.hum));
        frame->main_panel->m_textCO2->SetLabelText(wxString::Format(wxT("CO2: %i"), meas.co2));
        frame->main_panel->m_textVOC->SetLabelText(std::format("IAQ: {:.1f}, Gas: {:.1f}%", meas.voc, BsecHandler::Get()->GetGasPercentage()));
        frame->main_panel->m_textCO->SetLabelText(wxString::Format(wxT("CO: %i"), meas.co));
        frame->main_panel->m_textPM25->SetLabelText(wxString::Format(wxT("PM2.5: %i"), meas.pm25));
        frame->main_panel->m_textPM10->SetLabelText(wxString::Format(wxT("PM10: %i"), meas.pm10));
        frame->main_panel->m_textPressure->SetLabelText(wxString::Format(wxT("Pressure: %.1f"), meas.pressure));
        frame->main_panel->m_textR->SetLabelText(wxString::Format(wxT("R: %.1f"), meas.r));
        frame->main_panel->m_textG->SetLabelText(wxString::Format(wxT("G: %.1f"), meas.g));
        frame->main_panel->m_textB->SetLabelText(wxString::Format(wxT("B: %.1f"), meas.b));
        frame->main_panel->m_textLux->SetLabelText(wxString::Format(wxT("Lux: %i"), meas.lux));
        frame->main_panel->m_textCCT->SetLabelText(wxString::Format(wxT("CCT: %i"), meas.cct));
        frame->main_panel->m_textUV->SetLabelText(wxString::Format(wxT("UV: %i"), meas.uv));
        frame->main_panel->m_textTime->SetLabelText(wxString::Format(wxT("Time: %s"), std::format("{:%Y.%m.%d %H:%M:%OS} - {}", now, num_recv_meas)));
        frame->SetIconTooltip(wxString::Format(wxT("T: %.1f, H: %.1f, CO2: %d, IAQ: %.1f, CO: %d, PM2.5: %d, PM10: %d, Pressure: %.1f, Lux: %d, CCT: %d, UV: %d"),
            meas.temp, meas.hum, meas.co2, meas.voc, meas.co, meas.pm25, meas.pm10, meas.pressure, meas.lux, meas.cct, meas.uv));
    }
}
 
void Sensors::ResetMeasurements()
{
    Measurement m;
    num_recv_meas = 0;
    UpdateGui(m);
}

void Sensors::UpdateDatabaseIfNeeded()
{
    int64_t diff = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - DatabaseLogic::Get()->last_db_update).count();
    if(diff > m_GraphGenerationInterval)
        DatabaseLogic::Get()->GenerateGraphs();
    else
        WriteGraphs();

}

template<typename T> T Sensors::GetValueFromDequeue(const std::unique_ptr<Measurement>& meas, int offset)
{
    T retval = *(T*)(((char*)meas.get() + (char)offset));
    return retval;
}

template<int i, typename T1, typename T2> int Sensors::CalculateMinMaxAvg_Final(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset)
{
    //DBG("final %d, %p, %p\n", i, labels, data_values);
    WriteDataToHtmlFromContainer<T1, T2>(&labels[(ai - 1) - i], &data_values[(ai - 1) - i], &container[(ai - 1) - i], offset);
    return i;
}

template< int i, typename T1, typename T2> int Sensors::CalculateMinMaxAvg(int ai, std::string* labels, std::string* data_values, T2* container, size_t offset)
{
    if(i < ai) 
    {
        //DBG("generated function: %d, %d, %p, %p\n", ai, i, labels, data_values);
        /*return*/ CalculateMinMaxAvg_Final<i, T1>(ai, labels, data_values, container, offset);
    }
    {
        //DBG("r_dispatch labels %d, %d: result: %d\n\n", ai, i, ai - i);

        if constexpr(i > 0)
        {
            //DBG("bigger %p, %p, %p, %lld\n", labels, data_values, container, offset);
            //DBG("\n\nadd size: %lld\n\n", sizeof(std::string))
            return CalculateMinMaxAvg< i - 1, T1, T2 >(ai, labels, data_values, container, offset);
        }
        else
        {
            //DBG("smaller %d, %d\n", ai, i);
            return -1;
        }
    }
}

template<typename T1, typename T2> void Sensors::WriteDataToHtmlFromContainer(std::string* labels, std::string* data_values, T2* container, size_t offset)
{
    for(const auto& it : *container)
    {
        *labels += "'" + it->time + "',";
    }
    for(const auto& it : *container)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset);
        *data_values += std::to_string(val) + ",";
    }
}

#define CALCULATE_MINMAXAVG(labels, data, input, offset) \
    {\
        constexpr size_t arr_size = pp_sizeof_array(labels); \
        if constexpr (arr_size == 1) \
            CalculateMinMaxAvg<1, T1>(1, &labels, &data, &input, offset); \
        else\
            CalculateMinMaxAvg<arr_size, T1>(arr_size, labels, data, input, offset);\
    }

template<typename T1> void Sensors::WriteGraph(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1)
{
    if(template_str.empty())
        return; /* Do not proceed further when template.html is missing */

    std::scoped_lock lock(mtx);
    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
   
    std::string labels_time_last;
    std::string data_latest;
    std::string labels_time_day[3];
    std::string data_day[3];
    std::string labels_time_week[3];
    std::string data_week[3];
    
    try
    {
        CALCULATE_MINMAXAVG(labels_time_last, data_latest, last_meas, offset_1);
        CALCULATE_MINMAXAVG(labels_time_day, data_day, last_day, offset_1);
        CALCULATE_MINMAXAVG(labels_time_week, data_week, last_week, offset_1);
    }
    catch(...)
    {
        DBG("exception");
    }

    if(!out)
    {
        LOG(LogLevel::Error, "Failed to open {} for writing graphs!", std::string("Graphs/") + filename);
        return;
    }
#ifdef _WIN32  /* TODO: remove once std::format support is added to clang */
    try
    {
        std::string out_str = std::vformat(template_str, std::make_format_args(min_val, max_val,
            labels_time_last, "Latest " + std::string(name) + " readings", "window.chartColors.orange", "window.chartColors.orange", data_latest,
            labels_time_day[0],
            std::string(name) + " (Avg)", "window.chartColors.orange", "window.chartColors.orange", data_day[0],
            std::string(name) + " (Max)", "window.chartColors.blue", "window.chartColors.blue", data_day[1],
            std::string(name) + " (Min)", "window.chartColors.green", "window.chartColors.green", data_day[2],
            labels_time_week[0],
            std::string(name) + " (Avg)", "window.chartColors.orange", "window.chartColors.orange", data_week[0],
            std::string(name) + " (Max)", "window.chartColors.blue", "window.chartColors.blue", data_week[1],
            std::string(name) + " (Min)", "window.chartColors.green", "window.chartColors.green", data_week[2],
            "Last " + std::to_string(Sensors::Get()->GetGraphResolution()) + " measurements", "Last Day", "Last Week"));
        out << out_str;
        out.close();
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception: {}", e.what());
    }
#endif
}
