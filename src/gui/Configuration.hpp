#pragma once

#include "ConfigurationBackup.hpp"
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/treelist.h>
#include <wx/spinctrl.h>

template <class T>
class wxIntClientData : public wxClientData
{
public:
	wxIntClientData(T val) : value(val) {}
	~wxIntClientData() {}

	T GetValue() { return value; }
private:
	T value;
};

class MacroRecordBoxDialog : public wxDialog
{
public:
	MacroRecordBoxDialog(wxWindow* parent);

	void ShowDialog(const wxString& str);
	uint8_t GetType() { return m_RecordType->GetSelection(); }
	bool IsApplyClicked() { return m_IsApplyClicked; }
protected:
	void OnApply(wxCommandEvent& event);
private:
	wxRadioBox* m_RecordType = nullptr;
	wxStaticText* m_labelResult = nullptr;
	bool m_IsApplyClicked = false;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(MacroRecordBoxDialog);
};

class MacroAddBoxDialog : public wxDialog
{
public:
	MacroAddBoxDialog(wxWindow* parent);

	void ShowDialog(const wxString& macro_key, const wxString& macro_name);
	wxString GetMacroName() { return m_macroName->GetValue(); }
	wxString GetMacroKey() { return m_macroKey->GetValue(); }
	bool IsApplyClicked() { return m_IsApplyClicked; }
protected:
	void OnApply(wxCommandEvent& event);
private:
	wxTextCtrl* m_macroName = nullptr;
	wxTextCtrl* m_macroKey = nullptr;
	wxStaticText* m_labelResult = nullptr;
	bool m_IsApplyClicked = false;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(MacroAddBoxDialog);
};

class MacroEditBoxDialog : public wxDialog
{
public:
	MacroEditBoxDialog(wxWindow* parent);

	void ShowDialog(const wxString& str, uint8_t macro_type);
	uint8_t GetType() { return m_radioBox1->GetSelection(); }
	wxString GetText() { return m_textMsg->GetValue(); }
	bool IsApplyClicked() { return m_IsApplyClicked; }
protected:
	void OnApply(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
private:
	wxRadioBox* m_radioBox1 = nullptr;
	wxTextCtrl* m_textMsg = nullptr;
	wxStaticText* m_labelResult = nullptr;
	bool m_IsApplyClicked = false;
	wxTimer* m_timer = nullptr;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(MacroEditBoxDialog);
};

class ComTcpPanel : public wxScrolled<wxPanel>
{
public:
	ComTcpPanel(wxWindow* parent);
	void UpdatePanel();
	void OnPaint(wxPaintEvent& event);

	wxCheckBox* m_IsTcp = nullptr;
	wxSpinCtrl* m_TcpPortSpin = nullptr;
	wxCheckBox* m_IsPerAppMacro = nullptr;
	wxCheckBox* m_IsAdvancedMacro = nullptr;
	wxCheckBox* m_IsTcpForwarder = nullptr;
	wxTextCtrl* m_TcpForwarderIp = nullptr;
	wxSpinCtrl* m_TcpForwarderPort = nullptr;
	wxCheckBox* m_IsCom = nullptr;
	wxComboBox* m_serial = nullptr;
	wxCheckBox* m_IsCanSerial = nullptr;
	wxComboBox* m_CanSerial = nullptr;
	wxTextCtrl* m_CanDefaultTxList = nullptr;
	wxTextCtrl* m_CanDefaultRxList = nullptr;
	wxCheckBox*	m_SerialForwarderIsEnabled = nullptr;
	wxTextCtrl*	m_SerialForwarderBindIp = nullptr;
	wxSpinCtrl*	m_SerialForwarderPort = nullptr;
	wxTextCtrl* m_PathSepReplacerKey = nullptr;
	wxCheckBox* m_IsMinimizeOnExit = nullptr;
	wxCheckBox* m_IsMinimizeOnStartup = nullptr;
	wxCheckBox* m_RememberWindowSize = nullptr;
	wxCheckBox* m_AlwaysOnNumlock = nullptr;
	wxSpinCtrl* m_DefaultPage = nullptr;
	wxTextCtrl* m_ScreenshotKey = nullptr;
	wxTextCtrl* m_ScreenshotDateFmt = nullptr;
	wxTextCtrl* m_ScreenshotPath = nullptr;
	wxTextCtrl* m_BackupDateFmt = nullptr;
	wxCheckBox* m_IsSymlink = nullptr;
	wxTextCtrl* m_MarkSymlink = nullptr;
	wxTextCtrl* m_CreateSymlink = nullptr;
	wxTextCtrl* m_CreateHardlink = nullptr;
	wxCheckBox* m_IsAntiLock = nullptr;
	wxCheckBox* m_IsScreensSaverAfterLock = nullptr;
	wxSpinCtrl* m_AntiLockTimeout = nullptr;
	wxCheckBox* m_IsTerminalHotkey = nullptr;
	wxTextCtrl* m_TerminalHotkey = nullptr;

private:
	wxButton* m_Ok = nullptr;
	wxButton* m_Backup = nullptr;

	wxDECLARE_EVENT_TABLE();
};

class IKey;
class KeybrdPanel : public wxPanel
{
public:
	KeybrdPanel(wxWindow* parent);

	void OnTreeListChanged_Main(wxTreeListEvent& evt);
	void OnTreeListChanged_Details(wxTreeListEvent& evt);
	void OnItemContextMenu_Main(wxTreeListEvent& evt);
	void OnItemContextMenu_Details(wxTreeListEvent& evt);
	void OnItemActivated(wxTreeListEvent& event);
	void UpdateMainTree();
	void UpdateDetailsTree(std::unique_ptr<IKey>* ptr = nullptr);
	void OnKeyDown(wxKeyEvent& evt);

private:
	void ShowAddDialog();
	void ShowEditDialog(wxTreeListItem item);
	void DuplicateMacro(std::vector<std::unique_ptr<IKey>>& x, uint16_t id);
	void ManipulateMacro(std::vector<std::unique_ptr<IKey>>& x, uint16_t id, bool add);

	void TreeDetails_DeleteSelectedMacros();
	void TreeDetails_AddNewMacro();
	void TreeDetails_CloneMacro();
	void TreeDetails_MoveUpSelectedMacro();
	void TreeDetails_MoveDownSelectedMacro();
	void TreeDetails_StartRecording();

	std::vector<std::unique_ptr<IKey>>* GetKeyClassByItem(wxTreeListItem item, uint16_t& id);

	wxTreeListCtrl* tree = nullptr;
	wxTreeListCtrl* tree_details = nullptr;
	wxBitmapButton* btn_add = nullptr;
	wxBitmapButton* btn_record = nullptr;
	wxBitmapButton* btn_copy = nullptr;
	wxBitmapButton* btn_delete = nullptr;
	wxBitmapButton* btn_up = nullptr;
	wxBitmapButton* btn_down = nullptr;
	wxButton* m_Ok = nullptr;
	MacroRecordBoxDialog* record_dlg = nullptr;
	MacroEditBoxDialog* edit_dlg = nullptr;
	MacroAddBoxDialog* add_dlg = nullptr;

	wxString root_sel_str;
	wxString child_sel_str;
	wxDECLARE_EVENT_TABLE();
};

class ConfigurationPanel : public wxScrolled<wxPanel>
{
public:
	ConfigurationPanel(wxWindow* parent);
	void Changeing(wxAuiNotebookEvent& event);
	void UpdateSubpanels();

	wxAuiNotebook* m_notebook = nullptr;
	ComTcpPanel* comtcp_panel = nullptr;
	KeybrdPanel* keybrd_panel = nullptr;
	BackupPanel* backup_panel = nullptr;

private:
	void OnSize(wxSizeEvent& evt);
	wxDECLARE_EVENT_TABLE();
};