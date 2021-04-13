#include "Sensors.h"
#include "main_frame.h"
#include "Database.h"

#include <queue>

void Sensors::ProcessIncommingData(char* recv_data, const char* from_ip)
{
    float temp, hum;
    int send_interval, co2, voc, pm25, pm10, lux, cct;

    int ret = sscanf(recv_data, "%d|%f,%f,%d,%d,%d,%d,%d,%d,%*d,%*d,%*d", &send_interval, &temp, &hum, &co2, &voc, &pm25, &pm10, &lux, &cct);
    if(ret == 9)
    {
        std::shared_ptr<Measurement> m = std::make_shared<Measurement>(temp, hum, co2, voc, pm25, pm10, lux, cct);
        Database::Get()->InsertMeasurement(m);
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
        /*
        size_t size = last_meas.size();
        last_meas.push_back(std::move(m));
        if(size > 30)
            last_meas.pop_front();
            */
    }
    else
    {
        LOGMSG(warning, "Invalid data received from %s", from_ip);
    }
}
