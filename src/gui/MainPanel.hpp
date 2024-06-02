#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

class MainPanel : public wxPanel
{
public:
	MainPanel(wxFrame* parent);
	 
	void UpdateKeybindings();
	void UpdateStatuses();
	void UpdateCryptoPrices(float eth_buy, float eth_sell, float btc_buy, float btc_sell);

	wxButton* m_RefreshButton = nullptr;
	wxButton* m_ResetButton = nullptr;
	wxButton* m_GenerateGraphs = nullptr;
	wxButton* m_ClearMeasurements = nullptr;
	wxStaticText* m_textTemp = nullptr;
	wxStaticText* m_textHum = nullptr;
	wxStaticText* m_textCO2 = nullptr;
	wxStaticText* m_textVOC = nullptr;
	wxStaticText* m_textCO = nullptr;
	wxStaticText* m_textPM25 = nullptr;
	wxStaticText* m_textPM10 = nullptr;
	wxStaticText* m_textPressure = nullptr;
	wxStaticText* m_textR = nullptr;
	wxStaticText* m_textG = nullptr;
	wxStaticText* m_textB = nullptr;
	wxStaticText* m_textLux = nullptr;
	wxStaticText* m_textCCT = nullptr;
	wxStaticText* m_textUV = nullptr;
	wxStaticText* m_textTime = nullptr;
	wxButton* m_OpenGraphs = nullptr;
	wxSpinCtrl* m_GraphStartHours1 = nullptr;
	wxSpinCtrl* m_GraphStartHours2 = nullptr;
	wxStaticText* m_EthPrice = nullptr;
	wxStaticText* m_BtcPrice = nullptr;
	wxStaticText* m_WorkingDays = nullptr;
	wxButton* m_RefreshCrypto = nullptr;
	wxStaticText* m_WeekNumber = nullptr;

private:
	void UpdateCurrentWeekNumber();
	wxString FormatCurrentWeekNumber();

	std::map<std::string, wxStaticBox*> key_map;
	wxStaticText* m_CorsairDeviceName = nullptr;
	wxStaticLine* m_CorsairSeparatorLine_1 = nullptr;
	wxStaticLine* m_CorsairSeparatorLine_2 = nullptr;

	wxStaticText* m_TcpBackendStatus = nullptr;
	wxStaticText* m_KeyboardStatus = nullptr;
	wxStaticText* m_CanStatus = nullptr;
	wxStaticText* m_ModbusStatus = nullptr;
	wxDECLARE_EVENT_TABLE();
};