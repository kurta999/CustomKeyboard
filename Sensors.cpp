#include "Sensors.h"
#include "main_frame.h"
#include "Database.h"

#include <queue>
#include <fstream>
#include <iterator>

#include <fmt/format.h>

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
        
        std::shared_ptr<Measurement> m = std::make_shared<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct, std::move(fmt::format("{:%H:%M:%S}", *current_tm)));
        Database::Get()->InsertMeasurement(m);
        AddMeasurement(m);
        WriteGraphs();

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame)
        {
            frame->main_panel->m_textTemp->SetLabelText(wxString::Format(wxT("Temperature: %.2f"), m->temp));
            frame->main_panel->m_textHum->SetLabelText(wxString::Format(wxT("Humidity: %.2f"), m->hum));
            frame->main_panel->m_textCO2->SetLabelText(wxString::Format(wxT("CO2: %i"), m->co2));
            frame->main_panel->m_textVOC->SetLabelText(wxString::Format(wxT("VOC: %i"), m->voc));
            frame->main_panel->m_textPM25->SetLabelText(wxString::Format(wxT("PM25: %i"), m->pm25));
            frame->main_panel->m_textPM10->SetLabelText(wxString::Format(wxT("PM10: %i"), m->pm10));
            frame->main_panel->m_textLux->SetLabelText(wxString::Format(wxT("Lux: %i"), m->lux));
            frame->main_panel->m_textCCT->SetLabelText(wxString::Format(wxT("CCT: %i"), m->cct));
            frame->SetIconTooltip(wxString::Format(wxT("T: %.2f, H: %.2f, CO2: %d, VOC: %d, PM2.5: %d, PM10: %d, Lux: %d, CCT: %d"), m->temp, m->hum, m->co2, m->voc, m->pm25, m->pm10, m->lux, m->cct));
        }
    }
    else
    {
        LOGMSG(warning, "Invalid data received from {}", from_ip);
    }
}

template<typename T> T GetValueFromDequeue(std::shared_ptr<Measurement> meas, int offset)
{
    T retval = *(T*)((DWORD)meas.get() + (char)offset);
    return retval;
}

void Sensors::WriteGraphs()
{
    WriteGraphQueue<decltype(Measurement::temp)>("Temperature.html", 15, 40, "Temperature", offsetof(Measurement, temp));
    WriteGraphQueue<decltype(Measurement::hum)>("Humidity.html", 0, 100, "Humidity", offsetof(Measurement, hum));
    WriteGraphQueue<decltype(Measurement::co2)>("CO2.html", 200, 3000, "Temperature", offsetof(Measurement, co2));
    WriteGraphQueue<decltype(Measurement::voc)>("VOC.html", 0, 65535, "Temperature", offsetof(Measurement, voc));
    WriteGraphQueue<decltype(Measurement::pm25)>("PM25.html", 0, 1000, "Temperature", offsetof(Measurement, pm25));
    WriteGraphQueue<decltype(Measurement::pm10)>("PM10.html", 0, 1000, "Temperature", offsetof(Measurement, pm10));
    WriteGraphQueue<decltype(Measurement::lux)>("Lux.html", 0, 10000, "Temperature", offsetof(Measurement, lux));
    WriteGraphQueue<decltype(Measurement::cct)>("CCT.html", 0, 10000, "Temperature", offsetof(Measurement, cct));
}

template<typename T1> void Sensors::WriteGraphQueue(const char* filename, uint16_t min_val, uint16_t max_val, const char* name, size_t offset_1)
{
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs");

    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
   
    std::string labels_time_last;
    std::string data_latest;
    std::string labels_time_day;
    std::string data_day;
    std::string labels_time_week;
    std::string data_week;
    for(const auto& it : last_meas)
    {
        labels_time_last += "'" + it->time + "',";
    }
    for(const auto& it : last_meas)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_latest += std::to_string(val) + ",";
    }
    for(const auto& it : last_day)
    {
        labels_time_day += "'" + it->time + "',";
    }
    for(const auto& it : last_day)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_day += std::to_string(val) + ",";
    }
    for(const auto& it : last_week)
    {
        labels_time_week += "'" + it->time + "',";
    }
    for(const auto& it : last_week)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        data_week += std::to_string(val) + ",";
    }

    try
    {
        std::string out_str = std::move(fmt::format(template_str, min_val, max_val,
            labels_time_last, "Latest temperature readings", "window.chartColors.red", "window.chartColors.red", data_latest,
            labels_time_day,
            "Temperature (Avg)", "window.chartColors.red", "window.chartColors.red", data_day,
            "Temperature (Max)", "window.chartColors.blue", "window.chartColors.red", "0",
            "Temperature (Min)", "window.chartColors.green", "window.chartColors.red", "0",
            labels_time_week,
            "Temperature (Avg)", "window.chartColors.red", "window.chartColors.red", data_week,
            "Temperature (Max)", "window.chartColors.blue", "window.chartColors.red", "0",
            "Temperature (Min)", "window.chartColors.green", "window.chartColors.red", "0",
            "Last X", "Last Day", "Last Week"));
        out << out_str;
        out.close();
    }
    catch(std::exception& ex)
    {
        DBG("%s", ex.what());
    }
}

void Sensors::Init()
{
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs");

    std::ifstream t("Graphs/template.html", std::ifstream::binary);
    if(!t.is_open())
        throw fmt::format("missing template.html");

    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    t.seekg(0);
    template_str.resize(size);

    t.read(&template_str[0], size);
    t.close();
#if 0
    last_meas2.push_back(std::make_shared<Measurement>(0.34f, 1.2f, 22, 35, 4, 5, 6, 7, "aaa"));
    last_meas2.push_back(std::make_shared<Measurement>(0.74f, 2.2f, 23, 37, 4, 5, 6, 7, "aaa"));
    last_meas2.push_back(std::make_shared<Measurement>(0.94f, 3.2f, 24, 38, 4, 5, 6, 7, "aaa"));

    std::vector<std::string> labels{ "Januar", "Februar", "Marcius" };
    std::vector<int> values_1{ 20, 22, 21 };
    std::vector<int> values_2{ 40, 44, 46 };
    /*
    std::string temp = "Temperature";
    std::string hum = "Humidity";
    */
#endif
}



#if 0
template<typename T1, typename T2> void Sensors::WriteGraphQueue(const char* filename, const char* first_name, const char* second_name, size_t offset_1, size_t offset_2)
{
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs");

    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
    out << html_page_1;

    for(const auto& it : last_meas)
    {
        out << fmt::format("'{}',", it->time);
    }
    out << "], datasets: [{\nlabel: '" << first_name << "',\n";
    out << html_page_2;
    for(const auto& it : last_meas)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        out << val << ",";
    }
    out << "\n],\nyAxisID: 'y-axis-1',\n}, {\nlabel: '" << second_name << "',\n";

    out << html_page_3;
    for(const auto& it : last_meas)
    {
        T2 val = GetValueFromDequeue<T2>(it, offset_2);
        out << val << ",";
    }
    out << html_page_4;
    out.close();
}

template<typename T1, typename T2> void Sensors::WriteGraphVector(const char* filename, const char* first_name, const char* second_name, size_t offset_1, size_t offset_2, std::vector<std::shared_ptr<Measurement>>& vec)
{
    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
    out << html_page_1;

    for(const auto& it : vec)
    {
        out << fmt::format("'{}',", it->time);
    }
    out << "], datasets: [{\nlabel: '" << first_name << "',\n";
    out << html_page_2;
    for(const auto& it : vec)
    {
        T1 val = GetValueFromDequeue<T1>(it, offset_1);
        out << val << ",";
    }
    out << "\n],\nyAxisID: 'y-axis-1',\n}, {\nlabel: '" << second_name << "',\n";

    out << html_page_3;
    for(const auto& it : vec)
    {
        T2 val = GetValueFromDequeue<T2>(it, offset_2);
        out << val << ",";
    }
    out << html_page_4;
    out.close();
}
#endif
