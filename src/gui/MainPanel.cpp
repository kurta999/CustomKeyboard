#include "pch.hpp"

wxBEGIN_EVENT_TABLE(MainPanel, wxPanel)
wxEND_EVENT_TABLE()

MainPanel::MainPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* split_sizer = new wxBoxSizer(wxHORIZONTAL);
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
			DatabaseLogic::Get()->SetGraphHours(0, (uint32_t)m_GraphStartHours1->GetValue());
			DatabaseLogic::Get()->SetGraphHours(1, (uint32_t)m_GraphStartHours2->GetValue());
			DatabaseLogic::Get()->GenerateGraphs();
		});

#if defined _WIN32
#define ADD_MEASUREMENT_TEXT(var_name, html_name, name, tooltip, color) \
	var_name = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(-1, -1), 0); \
	var_name->Wrap(-1); \
	var_name->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
	var_name->SetForegroundColour(color); \
	var_name->SetToolTip(tooltip"\nPress MIDDLE CLICK to open graphs in browser"); \
	bSizer1->Add(var_name, 0, wxALL, 5); \
	var_name->Bind(wxEVT_MIDDLE_DOWN, [this](wxMouseEvent& event) \
		{ \
			DBGW(L"Graphs" ##html_name## ".html"); \
			ShellExecute(NULL, L"open", L"Graphs\\"##html_name##".html", NULL, NULL, SW_SHOWNORMAL); \
		})
#else
#define ADD_MEASUREMENT_TEXT(var_name, html_name, name, tooltip, color) \
	var_name = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(-1, -1), 0); \
	var_name->Wrap(-1); \
	var_name->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
	var_name->SetForegroundColour(color); \
	var_name->SetToolTip(tooltip"\nPress MIDDLE CLICK to open graphs in browser"); \
	bSizer1->Add(var_name, 0, wxALL, 5); 
#endif
	ADD_MEASUREMENT_TEXT(m_textTemp, "Temperature", "Temperature: N/A", "Unit: ?C", wxColour(244, 99, 11));
	ADD_MEASUREMENT_TEXT(m_textHum, "Humidity", "Humidity: N/A", "Unit: % RH", wxColour(29, 79, 252));
	ADD_MEASUREMENT_TEXT(m_textCO2, "CO2", "CO2: N/A", "Unit: ppm", wxColour(237, 60, 251));
	ADD_MEASUREMENT_TEXT(m_textVOC, "VOC", "VOC: N/A", "Unit: ppb", wxColour(185, 4, 200));
	ADD_MEASUREMENT_TEXT(m_textPM25, "PM25", "PM2.5: N/A", "Unit: ug/m3", wxColour(62, 211, 24));
	ADD_MEASUREMENT_TEXT(m_textPM10, "PM10", "PM10: N/A", "Unit: ug/m3", wxColour(50, 172, 19));
	ADD_MEASUREMENT_TEXT(m_textLux, "Lux", "Lux: N/A", "Unit: Lux", wxColour(84, 67, 71));
	ADD_MEASUREMENT_TEXT(m_textCCT, "CCT", "CCT: N/A", "Unit: Kelvin", wxColour(55, 45, 47));
	ADD_MEASUREMENT_TEXT(m_textTime, "CCT", "Time: N/A - 0", "Unit: Date & Time - Number of measurements received", wxColour(255, 21, 21));

	m_OpenGraphs = new wxButton(this, wxID_ANY, wxT("Open graphs"), wxDefaultPosition, wxDefaultSize, 0);
	m_OpenGraphs->SetToolTip("Open graphs file in browser");
	bSizer1->Add(m_OpenGraphs, 0, wxALL, 5);
	m_OpenGraphs->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
#ifdef _WIN32
			ShellExecute(NULL, L"open", L"Graphs\\Temperature.html", NULL, NULL, SW_SHOWNORMAL);
#else

#endif
		});

	wxStaticText* graph_helper_1 = new wxStaticText(this, wxID_ANY, "Hours backward for graph (1, 2) from db:", wxDefaultPosition, wxSize(-1, -1), 0); \
	graph_helper_1->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	bSizer1->Add(graph_helper_1, 0, wxALL, 5);
	m_GraphStartHours1 = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 16384, 1, 10 * 24 * 365); 
	m_GraphStartHours1->SetToolTip("Sets how many hours before the current time the first graph should be generated");
	m_GraphStartHours1->SetValue(DatabaseLogic::Get()->GetGraphHours(0));
	bSizer1->Add(m_GraphStartHours1, 0, wxALL, 5);

	m_GraphStartHours2 = new wxSpinCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 16384, 1, 10 * 24 * 365); 
	m_GraphStartHours2->SetToolTip("Sets how many hours before the current time the second graph should be generated");
	m_GraphStartHours2->SetValue(DatabaseLogic::Get()->GetGraphHours(1));
	bSizer1->Add(m_GraphStartHours2, 0, wxALL, 5);

	m_EthPrice = new wxStaticText(this, wxID_ANY, "ETH: 0.0 - 0.0", wxDefaultPosition, wxSize(-1, -1), 0);
	m_EthPrice->Wrap(-1);
	m_EthPrice->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	m_EthPrice->SetForegroundColour(wxColor(10, 104, 245));
	m_EthPrice->SetToolTip("ETH Buy & Sell [USD]");
	bSizer1->Add(m_EthPrice, 0, wxALL, 5);

	m_BtcPrice = new wxStaticText(this, wxID_ANY, "BTC: 0.0 - 0.0", wxDefaultPosition, wxSize(-1, -1), 0);
	m_BtcPrice->Wrap(-1);
	m_BtcPrice->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	m_BtcPrice->SetForegroundColour(wxColor(240, 120, 0));
	m_BtcPrice->SetToolTip("BTC Buy & Sell [USD]");
	bSizer1->Add(m_BtcPrice, 0, wxALL, 5);

	m_RefreshCrypto = new wxButton(this, wxID_ANY, wxT("Update"), wxDefaultPosition, wxDefaultSize, 0);
	m_RefreshCrypto->SetToolTip("Refresh crypto prices");
	bSizer1->Add(m_RefreshCrypto, 0, wxALL, 5);
	m_RefreshCrypto->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			CryptoPrice::Get()->UpdatePrices(true);
		});

	split_sizer->Add(bSizer1, wxSizerFlags(0).Top());

#define ADD_KEY(display_text, internal_name, key_name) \
	{\
		wxStaticBox* st_box = new wxStaticBox(this, wxID_ANY, wxT("N/A"));\
		key_map.emplace(internal_name, st_box); \
		wxStaticBoxSizer* num = new wxStaticBoxSizer(st_box, wxVERTICAL); \
		wxButton* button_num = new wxButton(num->GetStaticBox(), wxID_ANY, wxT(display_text), wxDefaultPosition, wxSize(50, 50), 0); \
		button_num->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
		num->Add(button_num, 0, wxALL, 5); \
		num_lock_box_sizer->Add(num, 1, wxEXPAND, 5); \
	}

	wxBoxSizer* second_vertical = new wxBoxSizer(wxVERTICAL);
	wxGridSizer* num_lock_box_sizer = new wxGridSizer(5, 4, 0, 0);

	ADD_KEY("NUM",		"NUM_LOCK",		lock);
	ADD_KEY("/",		"NUM_MUL",		mul);
	ADD_KEY("*",		"NUM_DIV",		div);
	ADD_KEY("-",		"NUM_MINUS",	minus);
	ADD_KEY("7",		"NUM_7",		7);
	ADD_KEY("8",		"NUM_8",		8);
	ADD_KEY("9",		"NUM_9",		9);
	ADD_KEY("+",		"NUM_PLUS",		plus);
	ADD_KEY("4",		"NUM_4",		4);
	ADD_KEY("5",		"NUM_5",		5);
	ADD_KEY("6",		"NUM_6",		6);
	ADD_KEY("<--",		"NUM_BACK", 	back);
	ADD_KEY("1",		"NUM_1",		1);
	ADD_KEY("2",		"NUM_2",		2);
	ADD_KEY("3",		"NUM_3",		3);
	ADD_KEY("E",	"NUM_ENTER",	enter1);
	ADD_KEY("0",		"NUM_0",		0);
	ADD_KEY("00",		"NUM_0",		00);
	ADD_KEY("DEL",		"NUM_DEL",		del);
	ADD_KEY("E",	"NUM_ENTER",	enter2);
	
	second_vertical->Add(num_lock_box_sizer);
	split_sizer->Add(second_vertical, wxSizerFlags(0));

	UpdateKeybindings();

	this->SetSizer(split_sizer);
}

void MainPanel::UpdateKeybindings()
{
	for(auto& i : key_map)
	{
		auto it = CustomMacro::Get()->GetMacros()[0]->key_vec.find(i.first);
		if(it != CustomMacro::Get()->GetMacros()[0]->key_vec.end())
		{
			i.second->SetLabelText(CustomMacro::Get()->GetMacros()[0]->bind_name[i.first]);
			i.second->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
			continue;
		}
		i.second->SetLabelText("N/A");
		i.second->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
		i.second->SetForegroundColour(*wxGREEN);
	}

	auto it = key_map.find(PrintScreenSaver::Get()->screenshot_key);
	if(it != key_map.end())
	{
		it->second->SetLabelText("SCREEN");
		it->second->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
		it->second->SetForegroundColour(*wxRED);
	}
	it = key_map.find(PathSeparator::Get()->replace_key);
	if(it != key_map.end())
	{
		it->second->SetLabelText("PATH");
		it->second->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
		it->second->SetForegroundColour(*wxRED);
	}
}

void MainPanel::UpdateCryptoPrices(float eth_buy, float eth_sell, float btc_buy, float btc_sell)
{
	m_EthPrice->SetLabelText(wxString::Format("ETH: %.1f - %.1f", eth_buy, eth_sell));
	m_BtcPrice->SetLabelText(wxString::Format("BTC: %.1f - %.1f", btc_buy, btc_sell));
}