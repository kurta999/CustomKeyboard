#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>

#include "ICmdHelper.hpp"

class CmdExecutorEditDialog : public wxDialog
{
public:
    CmdExecutorEditDialog(wxWindow* parent);

    void ShowDialog(const wxString& cmd_name, const wxString& cmd_to_execute, uint32_t color, uint32_t bg_color, bool is_bold, float scale);

    const wxString GetCmdName() { return m_commandName->GetValue(); }
    const wxString GetCmd() { return m_cmdToExecute->GetValue(); }

    const wxColor GetTextColor() { return m_color->GetColour(); }
    const wxColor GetBgColor() { return m_backgroundColor->GetColour(); }

    bool IsBold() { return m_isBold->GetValue(); }
    float GetScale() { return static_cast<float>(m_scale->GetValue()); }

    bool IsApplyClicked() { return m_IsApplyClicked; }
protected:
    void OnApply(wxCommandEvent& event);
    //void OnTimer(wxTimerEvent& event);
private:
    wxTextCtrl* m_commandName = nullptr;
    wxTextCtrl* m_cmdToExecute = nullptr;
    wxColourPickerCtrl* m_color = nullptr;
    wxColourPickerCtrl* m_backgroundColor = nullptr;
    wxCheckBox* m_isBold = nullptr;
    wxSpinCtrlDouble* m_scale = nullptr;

    wxStaticText* m_labelResult = nullptr;
    bool m_IsApplyClicked = false;
    wxTimer* m_timer = nullptr;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CmdExecutorEditDialog);
};

class CmdExecutorPanel : public wxPanel, public ICmdHelper
{
public:
	CmdExecutorPanel(wxFrame* parent);
	virtual ~CmdExecutorPanel() { }

	void ReloadCommands();

private:
	void ToggleAllButtonClickability(bool toggle);

	void OnSize(wxSizeEvent& evt);
	void OnPanelRightClick(wxMouseEvent& event);
	void OnClick(wxCommandEvent& event);
	void OnRightClick(wxMouseEvent& event);

	void OnPreReload(uint8_t cols) override;
	void OnCommandLoaded(uint8_t col, CommandTypes cmd) override;
	void OnPostReload(uint8_t cols) override;
	
	void AddCommandElement(uint8_t col, Command* c);
	void AddSeparatorElement(uint8_t col, Separator s);

    void UpdateCommandButon(Command* c, wxButton* btn, bool force_font_reset = false);
    void DeleteCommandButton(Command* c, wxButton* btn);

	wxGridSizer* m_BaseGrid = nullptr;
	std::vector<wxBoxSizer*> m_VertialBoxes;
	std::multimap<uint8_t, std::variant<wxButton*, wxStaticLine*>> m_ButtonMap;
	CmdExecutorEditDialog* edit_dlg = nullptr;

	wxDECLARE_EVENT_TABLE();
};