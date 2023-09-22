#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/bmpcbox.h>
#include <wx/artprov.h>

#include "ICmdHelper.hpp"

class CmdExecutorEditDialog : public wxDialog
{
public:
    CmdExecutorEditDialog(wxWindow* parent);

    void ShowDialog(const wxString& cmd_name, const wxString& cmd_to_execute, uint32_t color, uint32_t bg_color, bool is_bold, const wxString& font_face, float scale);

    const wxString GetCmdName() { return m_commandName->GetValue(); }
    const wxString GetCmd() { return m_cmdToExecute->GetValue(); }

    const wxColor GetTextColor() { return m_color->GetColour(); }
    const wxColor GetBgColor() { return m_backgroundColor->GetColour(); }

    bool IsBold() { return m_isBold->GetValue(); }
    wxString GetFontFace() { return m_fontFace->GetSelectedFont().GetFaceName(); }
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
    wxFontPickerCtrl* m_fontFace = nullptr;
    wxSpinCtrlDouble* m_scale = nullptr;

    wxStaticText* m_labelResult = nullptr;
    bool m_IsApplyClicked = false;
    wxTimer* m_timer = nullptr;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CmdExecutorEditDialog);
};

class CmdExecutorPanelPage;
class CmdExecutorPanelBase : public wxPanel, public ICmdHelper
{
public:
	CmdExecutorPanelBase(wxFrame* parent);
	virtual ~CmdExecutorPanelBase() { }

	void ReloadCommands();

    static inline uint8_t m_CurrentPage = 0;
private:
    void OnSize(wxSizeEvent& evt);
    void OnAuiRightClick(wxAuiNotebookEvent& evt);

	void OnPreReload(uint8_t page) override;
	void OnPreReloadColumns(uint8_t pages, uint8_t cols) override;
	void OnCommandLoaded(uint8_t page, uint8_t col, CommandTypes cmd) override;
	void OnPostReload(uint8_t page, uint8_t cols, CommandPageNames& names, CommandPageIcons& icons) override;
	
    void OnPanelRightClick(wxMouseEvent& event);

    wxAuiNotebook* m_notebook = nullptr;

    std::vector<CmdExecutorPanelPage*> m_Pages;

	wxDECLARE_EVENT_TABLE();
};

class CmdExecutorPanelPage : public wxPanel
{
public:
    CmdExecutorPanelPage(wxWindow* parent, uint8_t id, uint8_t cols);

    void OnPreload(uint8_t cols);
    void OnPostReloadUpdate();
    void OnCommandLoaded(uint8_t col, CommandTypes cmd);

    uint8_t m_Id = 0;

private:

    void ToggleAllButtonClickability(bool toggle);

    void OnSize(wxSizeEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnPanelRightClick(wxMouseEvent& event);
    void OnClick(wxCommandEvent& event);
    void OnRightClick(wxMouseEvent& event);
    void OnMiddleClick(wxMouseEvent& event);

    void AddCommandElement(uint8_t col, Command* c);
    void AddSeparatorElement(uint8_t col, Separator s);

    void UpdateCommandButon(Command* c, wxButton* btn, bool force_font_reset = false);
    void DeleteCommandButton(Command* c, wxButton* btn);

    void Execute(Command* c);

    CmdExecutorEditDialog* edit_dlg = nullptr;
    wxGridSizer* m_BaseGrid = nullptr;
    std::vector<wxStaticBoxSizer*> m_VertialBoxes;
    std::multimap<uint8_t, std::variant<wxButton*, wxStaticLine*>> m_ButtonMap;
    wxDECLARE_EVENT_TABLE();
};

static const wxString art_names[] = {
  "wxART_ADD_BOOKMARK",
  "wxART_DEL_BOOKMARK",
  "wxART_HELP_SIDE_PANEL",
  "wxART_HELP_SETTINGS",
  "wxART_HELP_BOOK",
  "wxART_HELP_FOLDER",
  "wxART_HELP_PAGE",
  "wxART_GO_BACK",
  "wxART_GO_FORWARD",
  "wxART_GO_UP",
  "wxART_GO_DOWN",
  "wxART_GO_TO_PARENT",
  "wxART_GO_HOME",
  "wxART_GOTO_FIRST",
  "wxART_GOTO_LAST",
  "wxART_FILE_OPEN",
  "wxART_FILE_SAVE",
  "wxART_FILE_SAVE_AS",
  "wxART_PRINT",
  "wxART_HELP",
  "wxART_TIP",
  "wxART_REPORT_VIEW",
  "wxART_LIST_VIEW",
  "wxART_NEW_DIR",
  "wxART_HARDDISK",
  "wxART_CDROM",
  "wxART_REMOVABLE",
  "wxART_FOLDER",
  "wxART_FOLDER_OPEN",
  "wxART_GO_DIR_UP",
  "wxART_EXECUTABLE_FILE",
  "wxART_NORMAL_FILE",
  "wxART_TICK_MARK",
  "wxART_CROSS_MARK",
  "wxART_ERROR",
  "wxART_QUESTION",
  "wxART_WARNING",
  "wxART_INFORMATION",
  "wxART_MISSING_IMAGE",
  "wxART_COPY",
  "wxART_CUT",
  "wxART_PASTE",
  "wxART_DELETE",
  "wxART_NEW",
  "wxART_UNDO",
  "wxART_REDO",
  "wxART_PLUS",
  "wxART_MINUS",
  "wxART_CLOSE",
  "wxART_QUIT",
  "wxART_FIND",
  "wxART_FIND_AND_REPLACE",
  "wxART_FULL_SCREEN",
  "wxART_EDIT",
  "wxART_WX_LOGO",
  "wxART_REFRESH",
  "wxART_STOP",
};

class IconSelectionDialog : public wxDialog {
public:
    IconSelectionDialog(wxWindow* parent);

    void SelectIconByName(const wxString& name);
    wxString GetSelectedIcon();

private:
    wxBitmapComboBox* icon_combo_box;
};