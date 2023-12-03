#include "pch.hpp"

wxBEGIN_EVENT_TABLE(MainPanel, wxPanel)
wxEND_EVENT_TABLE()

MainPanel::MainPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* split_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_RefreshButton = new wxButton(this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
	m_RefreshButton->SetToolTip("Request new measurements");
	horizontal_sizer->Add(m_RefreshButton);
	m_RefreshButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			for(auto ip : Server::Get()->used_ip_addresses)
			{
				std::string sensor_ip = boost::asio::ip::address_v4(ip).to_string();
				utils::SendTcpBlocking(sensor_ip, 80, "SEND_SENSOR_DATA", 16);
			}
		});

	m_ResetButton = new wxButton(this, wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0);
	m_ResetButton->SetToolTip("Restart device");
	horizontal_sizer->Add(m_ResetButton);
	m_ResetButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			for(auto ip : Server::Get()->used_ip_addresses)
			{
				std::string sensor_ip = boost::asio::ip::address_v4(ip).to_string();
				utils::SendTcpBlocking(sensor_ip, 80, "RESET", 5);
			}
		});
	bSizer1->Add(horizontal_sizer);

	wxBoxSizer* horizontal_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_GenerateGraphs = new wxButton(this, wxID_ANY, wxT("Generate graphs"), wxDefaultPosition, wxDefaultSize, 0);
	m_GenerateGraphs->SetToolTip("Generate graphs from SQLite database");
	horizontal_sizer_2->Add(m_GenerateGraphs);
	m_GenerateGraphs->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			DatabaseLogic::Get()->SetGraphHours(0, (uint32_t)m_GraphStartHours1->GetValue());
			DatabaseLogic::Get()->SetGraphHours(1, (uint32_t)m_GraphStartHours2->GetValue());
			DatabaseLogic::Get()->GenerateGraphs();
		});
	m_ClearMeasurements = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	m_ClearMeasurements->SetToolTip("Clear measurements");
	horizontal_sizer_2->Add(m_ClearMeasurements);
	m_ClearMeasurements->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Sensors::Get()->ResetMeasurements();
		});

	bSizer1->Add(horizontal_sizer_2);
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
	ADD_MEASUREMENT_TEXT(m_textTemp, "Temperature", "Temperature: N/A", "Unit: °C", wxColour(244, 99, 11));
	ADD_MEASUREMENT_TEXT(m_textHum, "Humidity", "Humidity: N/A", "Unit: % RH", wxColour(29, 79, 252));
	ADD_MEASUREMENT_TEXT(m_textCO2, "CO2", "CO2: N/A", "Unit: ppm", wxColour(237, 60, 251));
	ADD_MEASUREMENT_TEXT(m_textVOC, "VOC", "VOC: N/A", "Unit: ppb", wxColour(185, 4, 200));
	ADD_MEASUREMENT_TEXT(m_textCO, "CO", "CO: N/A", "Unit: ppb", wxColour(185, 4, 200));
	ADD_MEASUREMENT_TEXT(m_textPM25, "PM25", "PM2.5: N/A", "Unit: ug/m3", wxColour(62, 211, 24));
	ADD_MEASUREMENT_TEXT(m_textPM10, "PM10", "PM10: N/A", "Unit: ug/m3", wxColour(50, 172, 19));
	ADD_MEASUREMENT_TEXT(m_textPressure, "Pressure", "Pressure: N/A", "Unit: hPA/mbar", wxColour(44, 220, 230));
	ADD_MEASUREMENT_TEXT(m_textR, "R", "Lux: N/A", "Unit: Lux", wxColour(235, 52, 140));
	ADD_MEASUREMENT_TEXT(m_textG, "G", "Lux: N/A", "Unit: Lux", wxColour(235, 52, 140));
	ADD_MEASUREMENT_TEXT(m_textB, "B", "Lux: N/A", "Unit: Lux", wxColour(235, 52, 140));
	ADD_MEASUREMENT_TEXT(m_textLux, "Lux", "Lux: N/A", "Unit: Lux", wxColour(84, 67, 71));
	ADD_MEASUREMENT_TEXT(m_textCCT, "CCT", "CCT: N/A", "Unit: Kelvin", wxColour(55, 45, 47));
	ADD_MEASUREMENT_TEXT(m_textUV, "UV", "UV: N/A", "Unit: UV Index", wxColour(199, 192, 14));
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

	m_WeekNumber = new wxStaticText(this, wxID_ANY, FormatCurrentWeekNumber(), wxDefaultPosition, wxSize(-1, -1), 0);
	m_WeekNumber->SetToolTip("Week number in the year");
	m_WeekNumber->SetForegroundColour(*wxBLUE);
	m_WeekNumber->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	bSizer1->Add(m_WeekNumber, 0, wxALL, 5);

	split_sizer->Add(bSizer1, wxSizerFlags(0).Top());

#define ADD_KEY(display_text, internal_name, key_name) \
	{\
		wxStaticBox* st_box = new wxStaticBox(this, wxID_ANY, wxT("N/A"));\
		key_map.try_emplace(internal_name, st_box); \
		wxStaticBoxSizer* num = new wxStaticBoxSizer(st_box, wxVERTICAL); \
		wxButton* button_num = new wxButton(num->GetStaticBox(), wxID_ANY, wxT(display_text), wxDefaultPosition, wxSize(50, 50), 0); \
		button_num->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
		button_num->SetClientData((void*)internal_name); \
		button_num->SetToolTip("Click to execute binded macro"); \
		num->Add(button_num, 0, wxALL, 5); \
		num_lock_box_sizer->Add(num, 1, wxEXPAND, 5); \
		button_num->Bind(wxEVT_LEFT_DOWN, [this, &button_num](wxMouseEvent& event) \
		{ \
			wxButton* btn = dynamic_cast<wxButton*>(event.GetEventObject()); \
			if(!btn) return; \
			\
			char* data = reinterpret_cast<char*>(btn->GetClientData()); \
			if(!data) return; \
			CustomMacro::Get()->SimulateKeypress(data); \
		}); \
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
	ADD_KEY("E",		"NUM_ENTER",	enter1);
	ADD_KEY("0",		"NUM_0",		0);
	ADD_KEY("00",		"NUM_0",		00);
	ADD_KEY("DEL",		"NUM_DEL",		del);
	ADD_KEY("E",		"NUM_ENTER",	enter2);
	
	second_vertical->Add(num_lock_box_sizer);
	split_sizer->Add(second_vertical, wxSizerFlags(0));

	split_sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(3, 800), wxLI_VERTICAL), wxSizerFlags(0));

#define ADD_GKEY(display_text, internal_name, key_name, dest_sizer) \
	{\
		wxStaticBox* st_box = new wxStaticBox(this, wxID_ANY, wxT("N/A"));\
		key_map.try_emplace(internal_name, st_box); \
		wxStaticBoxSizer* num = new wxStaticBoxSizer(st_box, wxVERTICAL); \
		wxButton* button_num = new wxButton(num->GetStaticBox(), wxID_ANY, wxT(display_text), wxDefaultPosition, wxSize(50, 50), 0); \
		button_num->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
		button_num->SetClientData((void*)internal_name); \
		button_num->SetToolTip("Click to execute binded macro"); \
		num->Add(button_num, 0, wxALL, 5); \
		dest_sizer->Add(num, 1, wxEXPAND, 5); \
		button_num->Bind(wxEVT_LEFT_DOWN, [this, &button_num](wxMouseEvent& event) \
		{ \
			wxButton* btn = dynamic_cast<wxButton*>(event.GetEventObject()); \
			if(!btn) return; \
			\
			char* data = reinterpret_cast<char*>(btn->GetClientData()); \
			if(!data) return; \
			CustomMacro::Get()->SimulateKeypress(data); \
		}); \
	}
	
	wxBoxSizer* third_vertical = new wxBoxSizer(wxVERTICAL);

	m_CorsairDeviceName = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, -1), 0);
	m_CorsairDeviceName->SetForegroundColour(*wxBLUE);
	third_vertical->Add(m_CorsairDeviceName);
	third_vertical->AddSpacer(5);

	wxGridSizer* g_key_box_sizer_1 = new wxGridSizer(2, 3, 0, 0);
	ADD_GKEY("G1",	"G1",	g1, g_key_box_sizer_1);
	ADD_GKEY("G2",	"G2",	g2, g_key_box_sizer_1);
	ADD_GKEY("G3",	"G3",	g3, g_key_box_sizer_1);
	ADD_GKEY("G4",	"G4",	g4, g_key_box_sizer_1);
	ADD_GKEY("G5",	"G5",	g5, g_key_box_sizer_1);
	ADD_GKEY("G6",	"G6",	g6, g_key_box_sizer_1);
	third_vertical->Add(g_key_box_sizer_1);

	m_CorsairSeparatorLine_1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, 3), wxLI_HORIZONTAL);
	third_vertical->Add(m_CorsairSeparatorLine_1, wxSizerFlags(0));
	
	wxGridSizer* g_key_box_sizer_2 = new wxGridSizer(2, 3, 0, 0);
	ADD_GKEY("G7",	"G7",	g7, g_key_box_sizer_2);
	ADD_GKEY("G8",	"G8",	g8, g_key_box_sizer_2);
	ADD_GKEY("G9",	"G9",	g9, g_key_box_sizer_2);
	ADD_GKEY("G10", "G10",	g10, g_key_box_sizer_2);
	ADD_GKEY("G11",	"G11",	g11, g_key_box_sizer_2);
	ADD_GKEY("G12",	"G12",	g12, g_key_box_sizer_2);
	third_vertical->Add(g_key_box_sizer_2);

	m_CorsairSeparatorLine_2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, 3), wxLI_HORIZONTAL);
	third_vertical->Add(m_CorsairSeparatorLine_2, wxSizerFlags(0));

	wxGridSizer* g_key_box_sizer_3 = new wxGridSizer(2, 3, 0, 0);
	ADD_GKEY("G13", "G13",	g13, g_key_box_sizer_3);
	ADD_GKEY("G14", "G14",	g14, g_key_box_sizer_3);
	ADD_GKEY("G15", "G15",	g15, g_key_box_sizer_3);
	ADD_GKEY("G16", "G16",	g16, g_key_box_sizer_3);
	ADD_GKEY("G17", "G17",	g17, g_key_box_sizer_3);
	ADD_GKEY("G18", "G18",	g18, g_key_box_sizer_3);
	third_vertical->Add(g_key_box_sizer_3);

	split_sizer->Add(third_vertical, wxSizerFlags(0));

	// Statuses
	split_sizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(3, 800), wxLI_VERTICAL), wxSizerFlags(0));
	wxBoxSizer* v_status_sizer = new wxBoxSizer(wxVERTICAL);

	m_TcpBackendStatus = new wxStaticText(this, wxID_ANY, "TCP Backend: ", wxDefaultPosition, wxSize(-1, -1), 0);
	m_TcpBackendStatus->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	v_status_sizer->Add(m_TcpBackendStatus);

	m_KeyboardStatus = new wxStaticText(this, wxID_ANY, "Keyboard: ", wxDefaultPosition, wxSize(-1, -1), 0);
	m_KeyboardStatus->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	v_status_sizer->Add(m_KeyboardStatus);

	m_CanStatus = new wxStaticText(this, wxID_ANY, "CAN: ", wxDefaultPosition, wxSize(-1, -1), 0);
	m_CanStatus->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	v_status_sizer->Add(m_CanStatus);

	m_ModbusStatus = new wxStaticText(this, wxID_ANY, "Modbus: ", wxDefaultPosition, wxSize(-1, -1), 0);
	m_ModbusStatus->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	v_status_sizer->Add(m_ModbusStatus);

	split_sizer->Add(v_status_sizer);
	UpdateKeybindings();
	UpdateStatuses();

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

	auto MarkFunctionalKey = [this](wxStaticBox* box, const wxString& text)
		{
			box->SetLabelText(text);
			box->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
			box->SetForegroundColour(*wxRED);
		};


	auto it = key_map.find(PrintScreenSaver::Get()->screenshot_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "SCREEN");
	}
	it = key_map.find(PathSeparator::Get()->replace_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "PATH");
	}
	it = key_map.find(CustomMacro::Get()->bring_to_foreground_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "TOGGLE");
	}
	it = key_map.find(SymlinkCreator::Get()->mark_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "MARK");
	}
	it = key_map.find(SymlinkCreator::Get()->place_symlink_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "SYMLINK");
	}	it = key_map.find(SymlinkCreator::Get()->place_hardlink_key);
	if(it != key_map.end())
	{
		MarkFunctionalKey(it->second, "HARDLINK");
	}

	if(CorsairHid::Get()->GetDeviceType() == CorsairDeviceType::NONE)
	{
		if(key_map["G18"]->IsShown())
		{
			for(uint8_t i = 0; i <= 18; i++)
			{
				key_map["G" + std::to_string(i)]->Hide();
			}
		}

		m_CorsairSeparatorLine_1->Hide();
		m_CorsairSeparatorLine_2->Hide();
	}
	else if(CorsairHid::Get()->GetDeviceType() == CorsairDeviceType::K95_PLATINUM)
	{
		if(key_map["G18"]->IsShown())
		{
			for(uint8_t i = 7; i <= 18; i++)
			{
				key_map["G" + std::to_string(i)]->Hide();
			}
		}

		m_CorsairSeparatorLine_1->Hide();
		m_CorsairSeparatorLine_2->Hide();
	}
	else if(CorsairHid::Get()->GetDeviceType() == CorsairDeviceType::K95_18GKEY)
	{
		if(!key_map["G18"]->IsShown())
		{
			for(uint8_t i = 7; i <= 18; i++)
			{
				key_map["G" + std::to_string(i)]->Show();
			}
		}

		m_CorsairSeparatorLine_1->Show();
		m_CorsairSeparatorLine_2->Show();
	}

	m_CorsairDeviceName->SetLabelText(wxString::Format("%s - %s", CorsairHid::Get()->GetDeviceName(), CorsairHid::Get()->IsOk() ? "OK" : "ERROR"));
	m_CorsairDeviceName->SetForegroundColour(CorsairHid::Get()->IsOk() ? *wxBLUE : *wxRED);
}

void MainPanel::UpdateStatuses()
{
	if(Server::Get()->is_enabled)
	{
		if(Server::Get()->is_ok)
		{
			m_TcpBackendStatus->SetLabelText("TCP: OK");
			m_TcpBackendStatus->SetForegroundColour(*wxGREEN);
		}
		else
		{
			m_TcpBackendStatus->SetLabelText("TCP: ERR");
			m_TcpBackendStatus->SetForegroundColour(*wxRED);
		}
	}
	else
	{
		m_TcpBackendStatus->SetLabelText("TCP: OFF");
		m_TcpBackendStatus->SetForegroundColour(*wxBLUE);
	}

	if(SerialPort::Get()->IsEnabled())
	{
		if(SerialPort::Get()->IsOk())
		{
			m_KeyboardStatus->SetLabelText("Keyboard: OK");
			m_KeyboardStatus->SetForegroundColour(*wxGREEN);
		}
		else
		{
			m_KeyboardStatus->SetLabelText("Keyboard: ERR");
			m_KeyboardStatus->SetForegroundColour(*wxRED);
		}
	}
	else
	{
		m_KeyboardStatus->SetLabelText("Keyboard: OFF");
		m_KeyboardStatus->SetForegroundColour(*wxBLUE);
	}

	if(CanSerialPort::Get()->IsEnabled())
	{
		if(CanSerialPort::Get()->IsOk())
		{
			m_CanStatus->SetLabelText("CAN: OK");
			m_CanStatus->SetForegroundColour(*wxGREEN);
		}
		else
		{
			m_CanStatus->SetLabelText("CAN: ERR");
			m_CanStatus->SetForegroundColour(*wxRED);
		}
	}
	else
	{
		m_CanStatus->SetLabelText("CAN: OFF");
		m_CanStatus->SetForegroundColour(*wxBLUE);
	}

	std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
	if(modbus_handler->GetSerial().IsEnabled())
	{
		if(CanSerialPort::Get()->IsOk())
		{
			m_ModbusStatus->SetLabelText("Modbus: OK");
			m_ModbusStatus->SetForegroundColour(*wxGREEN);
		}
		else
		{
			m_ModbusStatus->SetLabelText("Modbus: ERR");
			m_ModbusStatus->SetForegroundColour(*wxRED);
		}
	}
	else
	{
		m_ModbusStatus->SetLabelText("Modbus: OFF");
		m_ModbusStatus->SetForegroundColour(*wxBLUE);
	}
}

void MainPanel::UpdateCryptoPrices(float eth_buy, float eth_sell, float btc_buy, float btc_sell)
{
	if(Settings::Get()->crypto_price_update != 0)
	{
		m_EthPrice->SetLabelText(wxString::Format("ETH: %.1f - %.1f", eth_buy, eth_sell));
		m_BtcPrice->SetLabelText(wxString::Format("BTC: %.1f - %.1f", btc_buy, btc_sell));
	}
	else
	{
		m_EthPrice->SetLabelText(wxString::Format("ETH: N/A - N/A", eth_buy, eth_sell));
		m_BtcPrice->SetLabelText(wxString::Format("BTC: N/A - N/A", btc_buy, btc_sell));
	}
	UpdateCurrentWeekNumber();
}

wxString MainPanel::FormatCurrentWeekNumber()
{
	boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
	int week_number = current_date.week_number();
	wxString week_str = wxString::Format("Week: %d", week_number);
	return week_str;
}

void MainPanel::UpdateCurrentWeekNumber()
{
	m_WeekNumber->SetLabelText(FormatCurrentWeekNumber());
}