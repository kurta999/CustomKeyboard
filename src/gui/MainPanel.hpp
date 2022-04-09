#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>

class MainPanel : public wxPanel
{
public:
	MainPanel(wxFrame* parent);
	 
	void UpdateKeybindings();

	wxButton* m_RefreshButton = nullptr;
	wxButton* m_GenerateGraphs = nullptr;
	wxStaticText* m_textTemp;
	wxStaticText* m_textHum;
	wxStaticText* m_textCO2;
	wxStaticText* m_textVOC;
	wxStaticText* m_textPM25;
	wxStaticText* m_textPM10;
	wxStaticText* m_textLux;
	wxStaticText* m_textCCT;
	wxStaticText* m_textTime;
	wxButton* m_OpenGraphs = nullptr;
	wxSpinCtrl* m_GraphStartHours1;
	wxSpinCtrl* m_GraphStartHours2;

private:
	std::map<std::string, wxStaticBox*> key_map;

	wxDECLARE_EVENT_TABLE();
};