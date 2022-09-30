#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

#include "ICmdHelper.hpp"

class CmdExecutorPanel : public wxPanel, public ICmdHelper
{
public:
	CmdExecutorPanel(wxFrame* parent);
	virtual ~CmdExecutorPanel() { }

	void ReloadCommands();

private:
	void ToggleAllButtonClickability(bool toggle);

	void OnSize(wxSizeEvent& evt);
	void OnClick(wxCommandEvent& event);

	void OnPreReload(uint8_t cols) override;
	void OnCommandLoaded(uint8_t col, CommandTypes cmd) override;
	void OnPostReload(uint8_t cols) override;

	wxGridSizer* m_BaseGrid = nullptr;
	std::vector<wxBoxSizer*> m_VertialBoxes;
	std::multimap<uint8_t, std::variant<wxButton*, wxStaticLine*>> m_ButtonMap;

	wxDECLARE_EVENT_TABLE();
};