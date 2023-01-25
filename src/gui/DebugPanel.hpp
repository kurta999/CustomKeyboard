#pragma once

#include <wx/wx.h>

class DebugPanel : public wxPanel
{
public:
	DebugPanel(wxFrame* parent);
	
	void HandleUpdate();

	wxStaticText* m_MousePos = nullptr;
	wxStaticText* m_ActiveWindowTitle = nullptr;
	wxButton* m_SaveScreenshot = nullptr;
	wxButton* m_PathSeparatorReplace = nullptr;

	wxTextCtrl* m_KeyTopress = nullptr;
	wxButton* m_SimulateKeypress = nullptr;

	wxTextCtrl* m_TextPanel = nullptr;

	wxSpinCtrl* m_CpuMinPowerPercent = nullptr;
	wxSpinCtrl* m_CpuMaxPowerPercent = nullptr;
	wxButton* m_CpuPowerRefresh = nullptr;
	wxButton* m_CpuPowerApply = nullptr;
private:

	std::future<void> keypress_future;
	wxDECLARE_EVENT_TABLE();
};
