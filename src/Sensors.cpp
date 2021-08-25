#include "pch.h"

void Sensors::ProcessIncommingData(char* recv_data, const char* from_ip)
{
    float temp, hum;
    int send_interval, co2, voc, pm25, pm10, lux, cct;

    int ret = sscanf(recv_data, "%d|%f,%f,%d,%d,%d,%d,%d,%d,%*d,%*d,%*d", &send_interval, &temp, &hum, &co2, &voc, &pm25, &pm10, &lux, &cct);
    if(ret == 9)
    {
        time_t current_time;
        tm* current_tm;
        time(&current_time);
        current_tm = localtime(&current_time);
        
        std::unique_ptr<Measurement> m = std::make_unique<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(fmt::format("{:%H:%M:%S}", *current_tm)));
        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
        {
            frame->main_panel->m_textTemp->SetLabelText(wxString::Format(wxT("Temperature: %.1f"), m->temp));
            frame->main_panel->m_textHum->SetLabelText(wxString::Format(wxT("Humidity: %.1f"), m->hum));
            frame->main_panel->m_textCO2->SetLabelText(wxString::Format(wxT("CO2: %i"), m->co2));
            frame->main_panel->m_textVOC->SetLabelText(wxString::Format(wxT("VOC: %i"), m->voc));
            frame->main_panel->m_textPM25->SetLabelText(wxString::Format(wxT("PM2.5: %i"), m->pm25));
            frame->main_panel->m_textPM10->SetLabelText(wxString::Format(wxT("PM10: %i"), m->pm10));
            frame->main_panel->m_textLux->SetLabelText(wxString::Format(wxT("Lux: %i"), m->lux));
            frame->main_panel->m_textCCT->SetLabelText(wxString::Format(wxT("CCT: %i"), m->cct));
            frame->main_panel->m_textTime->SetLabelText(wxString::Format(wxT("Time: %s"), fmt::format("{:%Y.%m.%d %H:%M:%S} - {}", *current_tm, ++num_recv_meas)));
            frame->SetIconTooltip(wxString::Format(wxT("T: %.1f, H: %.1f, CO2: %d, VOC: %d, PM2.5: %d, PM10: %d, Lux: %d, CCT: %d"), m->temp, m->hum, m->co2, m->voc, m->pm25, m->pm10, m->lux, m->cct));
        }

        Database::Get()->InsertMeasurement(m);
        AddMeasurement(std::move(m));
        if(current_time - Database::Get()->last_db_update > 10 * 60)  /* 10 minutes */
            Database::Get()->GenerateGraphs();
        else
            WriteGraphs();
    }
    else
    {
        LOGMSG(warning, "Invalid data received from {}", from_ip);
    }
}

template<typename T> T GetValueFromDequeue(const std::unique_ptr<Measurement>& meas, int offset)
{
    T retval = *(T*)(((char*)meas.get() + (char)offset));
    return retval;
}

void Sensors::WriteGraphs()
{
    WriteGraph<decltype(Measurement::temp)>("Temperature.html", 15, 35, "Temperature", offsetof(Measurement, temp));
    WriteGraph<decltype(Measurement::hum)>("Humidity.html", 0, 100, "Humidity", offsetof(Measurement, hum));
    WriteGraph<decltype(Measurement::co2)>("CO2.html", 150, 3000, "CO2", offsetof(Measurement, co2));
    WriteGraph<decltype(Measurement::voc)>("VOC.html", 0, 65535, "VOC", offsetof(Measurement, voc));
    WriteGraph<decltype(Measurement::pm25)>("PM25.html", 0, 1000, "PM2.5", offsetof(Measurement, pm25));
    WriteGraph<decltype(Measurement::pm10)>("PM10.html", 0, 1000, "PM10", offsetof(Measurement, pm10));
    WriteGraph<decltype(Measurement::lux)>("Lux.html", 0, 10000, "Lux", offsetof(Measurement, lux));
    WriteGraph<decltype(Measurement::cct)>("CCT.html", 0, 10000, "CCT", offsetof(Measurement, cct));
}

template<typename T1> void Sensors::WriteGraph(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1)
{
    std::scoped_lock lock(mtx);
    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
   
    std::string labels_time_last;
    std::string data_latest;
    std::string labels_time_day[3];
    std::string data_day[3];
    std::string labels_time_week[3];
    std::string data_week[3];
    
    for(const auto& it : last_meas)
    {
        labels_time_last += "'" + it->time + "',";
    }
    for(const auto& it : last_meas)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_latest += std::to_string(val) + ",";
    }
    for(const auto& it : last_day[0])
    {
        labels_time_day[0] += "'" + it->time + "',";
    }
    for(const auto& it : last_day[0])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_day[0] += std::to_string(val) + ",";
    }
    for(const auto& it : last_day[1])
    {
        labels_time_day[1] += "'" + it->time + "',";
    }
    for(const auto& it : last_day[1])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_day[1] += std::to_string(val) + ",";
    }
    for(const auto& it : last_day[2])
    {
        labels_time_day[2] += "'" + it->time + "',";
    }
    for(const auto& it : last_day[2])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_day[2] += std::to_string(val) + ",";
    }

    for(const auto& it : last_week[0])
    {
        labels_time_week[0] += "'" + it->time + "',";
    }
    for(const auto& it : last_week[0])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_week[0] += std::to_string(val) + ",";
    }
    for(const auto& it : last_week[1])
    {
        labels_time_week[1] += "'" + it->time + "',";
    }
    for(const auto& it : last_week[1])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_week[1] += std::to_string(val) + ",";
    }
    for(const auto& it : last_week[2])
    {
        labels_time_week[2] += "'" + it->time + "',";
    }
    for(const auto& it : last_week[2])
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_week[2] += std::to_string(val) + ",";
    }
    
    try
    {
        std::string out_str = fmt::format(template_str, min_val, max_val,
            labels_time_last, "Latest " + std::string(name) + " readings", "window.chartColors.orange", "window.chartColors.orange", data_latest,
            labels_time_day[0],
            std::string(name) + " (Avg)", "window.chartColors.orange", "window.chartColors.orange", data_day[0],
            std::string(name) + " (Max)", "window.chartColors.blue", "window.chartColors.blue", data_day[1],
            std::string(name) + " (Min)", "window.chartColors.green", "window.chartColors.green", data_day[2],
            labels_time_week[0],
            std::string(name) + " (Avg)", "window.chartColors.orange", "window.chartColors.orange", data_week[0],
            std::string(name) + " (Max)", "window.chartColors.blue", "window.chartColors.blue", data_week[1],
            std::string(name) + " (Min)", "window.chartColors.green", "window.chartColors.green", data_week[2],
            "Last " + std::to_string(MAX_MEAS_QUEUE) + " measurements", "Last Day", "Last Week");
        out << out_str;
        out.close();
    }
    catch(std::exception& e)
    {
        LOGMSG(critical, "Exception %s", e.what());
    }
}

void Sensors::Init()
{
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs");

    std::ifstream t("Graphs/template.html", std::ifstream::binary);
    if(!t.is_open())
        throw std::runtime_error(fmt::format("Missing template.html"));

    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    t.seekg(0);
    template_str.resize(size);

    t.read(&template_str[0], size);
    t.close();
}