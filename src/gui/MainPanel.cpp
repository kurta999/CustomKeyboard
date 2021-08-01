#include "pch.h"

MainPanel::MainPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_RefreshButton = new wxButton(this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
	m_RefreshButton->SetToolTip("Request new measurements");
	bSizer1->Add(m_RefreshButton, 0, wxALL, 5);
	m_RefreshButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Server::Get()->BroadcastMessage("SEND_SENSOR_DATA");
		});

	m_GenerateGraphs = new wxButton(this, wxID_ANY, wxT("Generate graphs"), wxDefaultPosition, wxDefaultSize, 0);
	m_GenerateGraphs->SetToolTip("Generate graphs from SQLite database");
	bSizer1->Add(m_GenerateGraphs, 0, wxALL, 5);
	m_GenerateGraphs->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Database::Get()->SetGraphHours(0, (uint32_t)m_GraphStartHours1->GetValue());
			Database::Get()->SetGraphHours(1, (uint32_t)m_GraphStartHours2->GetValue());
			Database::Get()->GenerateGraphs();
		});


#define ADD_MEASUREMENT_TEXT(var_name, name, tooltip) \
	var_name = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(-1, -1), 0); \
	var_name->Wrap(-1); \
	var_name->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
	var_name->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND)); \
	var_name->SetToolTip(tooltip); \
	bSizer1->Add(var_name, 0, wxALL, 5)

	ADD_MEASUREMENT_TEXT(m_textTemp, "Temperature: N/A", "Unit: 0,1°C");
	ADD_MEASUREMENT_TEXT(m_textHum, "Humidity: N/A", "Unit: 0,1%RH");
	ADD_MEASUREMENT_TEXT(m_textCO2, "CO2: N/A", "Unit: ppm");
	ADD_MEASUREMENT_TEXT(m_textVOC, "VOC: N/A", "Unit: ppb");
	ADD_MEASUREMENT_TEXT(m_textPM25, "PM2.5: N/A", "Unit: ug/m3");
	ADD_MEASUREMENT_TEXT(m_textPM10, "PM10: N/A", "Unit: ug/m3");
	ADD_MEASUREMENT_TEXT(m_textLux, "Lux: N/A", "Unit: Lux");
	ADD_MEASUREMENT_TEXT(m_textCCT, "CCT: N/A", "Unit: Kelvin");
	ADD_MEASUREMENT_TEXT(m_textTime, "Time: N/A", "Unit: Date & Time");

	m_OpenGraphs = new wxButton(this, wxID_ANY, wxT("Open graphs"), wxDefaultPosition, wxDefaultSize, 0);
	m_OpenGraphs->SetToolTip("Open graphs file in browser");
	bSizer1->Add(m_OpenGraphs, 0, wxALL, 5);
	m_OpenGraphs->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			ShellExecute(NULL, L"open", L"Graphs\\Temperature.html", NULL, NULL, SW_SHOWNORMAL);
		});

	m_GraphStartHours1 = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 16384, 1, 7 * 24);
	m_GraphStartHours1->SetToolTip("Sets how many hours before current time the first graph should start");
	m_GraphStartHours1->SetValue(Database::Get()->GetGraphHours(0));
	bSizer1->Add(m_GraphStartHours1, 0, wxALL, 5);
	m_GraphStartHours2 = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 16384, 1, 365 * 24);
	m_GraphStartHours2->SetToolTip("Sets how many hours before current time the second graph should start");
	m_GraphStartHours2->SetValue(Database::Get()->GetGraphHours(1));
	bSizer1->Add(m_GraphStartHours2, 0, wxALL, 5);

	this->SetSizer(bSizer1);
}
