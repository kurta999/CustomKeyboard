#include "Sensors.h"
#include "main_frame.h"
#include "Database.h"

#include <queue>
#include <fstream>
#include <iterator>

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

const char* html_page_1= "<!DOCTYPE html>\n\
<!-- saved from url=(0066)https://www.chartjs.org/samples/latest/charts/line/multi-axis.html -->\n\
<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n\
	<title>Line Chart Multiple Axes</title>\n\
	<script async=\"\" src=\"./Line Chart Multiple Axes_files/analytics.js.download\"></script><script src=\"./Line Chart Multiple Axes_files/Chart.min.js.download\"></script>\n\
	<script src=\"./Line Chart Multiple Axes_files/utils.js.download\"></script>\n\
	<style>\n\
	canvas {\n\
		-moz-user-select: none;\n\
		-webkit-user-select: none;\n\
		-ms-user-select: none;\n\
	}\
	</style>\n\
<style type=\"text/css\">/* Chart.js */\n\
@keyframes chartjs-render-animation{from{opacity:.99}to{opacity:1}}.chartjs-render-monitor{animation:chartjs-render-animation 1ms}.chartjs-size-monitor,.chartjs-size-monitor-expand,.chartjs-size-monitor-shrink{position:absolute;direction:ltr;left:0;top:0;right:0;bottom:0;overflow:hidden;pointer-events:none;visibility:hidden;z-index:-1}.chartjs-size-monitor-expand>div{position:absolute;width:1000000px;height:1000000px;left:0;top:0}.chartjs-size-monitor-shrink>div{position:absolute;width:200%;height:200%;left:0;top:0}</style></head\n>\
\n\
<body>\n\
	<div style=\"width:75%;\"><div class=\"chartjs-size-monitor\"><div class=\"chartjs-size-monitor-expand\"><div class=\"\"></div></div><div class=\"chartjs-size-monitor-shrink\"><div class=\"\"></div></div></div>\n\
		<canvas id=\"canvas\" style=\"display: block; width: 1428px; height: 714px;\" width=\"1428\" height=\"714\" class=\"chartjs-render-monitor\"></canvas>\n\
	</div>\n\
	<script>\n\
		var lineChartData = {\n\
        labels: [\n";

const char* html_page_2 = "\
borderColor : window.chartColors.red,\n\
backgroundColor : window.chartColors.red,\n\
fill : false,\n\
data : [\n";

const char* html_page_3 = "\
                borderColor : window.chartColors.blue,\n\
                backgroundColor : window.chartColors.blue,\n\
                fill : false,\n\
                data : [\n"; \


    const char* html_page_4 = "\n],\
yAxisID: 'y-axis-2'\n\
            }]\n\
        };\n\
\n\
        window.onload = function() {\n\
            var ctx = document.getElementById('canvas').getContext('2d');\n\
            window.myLine = Chart.Line(ctx, {\n\
                data: lineChartData,\n\
                options: {\n\
                    responsive: true,\n\
                    hoverMode: 'index',\n\
                    stacked: false,\n\
                    title: {\n\
                        display: true,\n\
                        text: 'Chart.js Line Chart - Multi Axis'\n\
                    },\n\
                    scales: {\n\
                        yAxes: [{\n\
                            type: 'linear',\n\
                            display: true,\n\
                            position: 'left',\n\
                            id: 'y-axis-1',\n\
                        },\n\
                        {\n\
                            type: 'linear',\n\
                            display: true,\n\
                            position: 'right',\n\
                            id: 'y-axis-2',\n\
\n\
                            gridLines: {\n\
                                drawOnChartArea: false,\n\
                            },\n\
                        }],\n\
                    }\n\
                }\n\
            });\n\
        };\n\
        </script>\n\
            </body> </html>";

template<typename T> T GetValueFromDequeue(std::shared_ptr<Measurement> meas, int offset)
{
    T retval = *(T*)((DWORD)meas.get() + (char)offset);
    return retval;
}

void Sensors::WriteGraphs()
{
    WriteGraph<float, float>("TempHum.html", "Temperature", "Humidity", offsetof(Measurement, temp), offsetof(Measurement, hum));
    WriteGraph<int, int>("CO2VOC.html", "CO2", "VOC", offsetof(Measurement, co2), offsetof(Measurement, voc));
    WriteGraph<int, int>("Particle.html", "PM2.5", "PM1.0", offsetof(Measurement, pm25), offsetof(Measurement, pm10));
    WriteGraph<int, int>("Light.html", "Lux", "CCT", offsetof(Measurement, lux), offsetof(Measurement, cct));

}

template<typename T1, typename T2> void Sensors::WriteGraph(const char* filename, const char* first_name, const char* second_name, size_t offset_1, size_t offset_2)
{
    if(!std::filesystem::exists("Graphs"))
        std::filesystem::create_directory("Graphs");

    std::ofstream out(std::string("Graphs/") + filename, std::ofstream::binary);
    out << html_page_1;

    for(const auto &it : last_meas)
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


void Sensors::Init()
{
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