#pragma once

#include "IModbusEntry.hpp"
#include <wx/tipwin.h>

enum ModbusGridCol : int
{
    Modbus_Name,
    Modbus_Value,
	Modbus_Max
};

enum ModbusLogGridCol : int
{
	ModbusLog_Time,
	ModbusLog_Direction,
	ModbusLog_FCode,
	ModbusLog_ErrorType,
	ModbusLog_DataSize,
	ModbusLog_Data,
	ModbusLog_Max
};

class ModbusDataEditDialog : public wxDialog
{
public:
	ModbusDataEditDialog(wxWindow* parent);

	void ShowDialog(std::optional<uint32_t> color, std::optional<uint32_t> bg_color, bool is_bold, const wxString& font_face, float scale);

	std::optional<uint32_t> GetTextColor();
	std::optional<uint32_t> GetBgColor();

	bool IsBold() { return m_isBold->GetValue(); }
	wxString GetFontFace() { return m_fontFace->GetSelectedFont().GetFaceName(); }
	float GetScale() { return static_cast<float>(m_scale->GetValue()); }

	bool IsApplyClicked() { return m_IsApplyClicked; }
protected:
	void OnApply(wxCommandEvent& event);
	//void OnTimer(wxTimerEvent& event);
private:
	wxCheckBox* m_useCustomColor = nullptr;
	wxColourPickerCtrl* m_color = nullptr;
	wxCheckBox* m_useCustomBackgroundColor = nullptr;
	wxColourPickerCtrl* m_backgroundColor = nullptr;
	wxCheckBox* m_isBold = nullptr;
	wxFontPickerCtrl* m_fontFace = nullptr;
	wxSpinCtrlDouble* m_scale = nullptr;

	wxStaticText* m_labelResult = nullptr;
	bool m_IsApplyClicked = false;
	wxTimer* m_timer = nullptr;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(ModbusDataEditDialog);
};

class ModbusItemPanel
{
public:
    ModbusItemPanel(wxWindow* parent, ModbusDataEditDialog* style_dialog, const wxString& header_name, ModbusItemType& items, bool is_read_only);

	void AddItem(std::unique_ptr<ModbusItem>& e);
    void UpdatePanel();
    void UpdateChangesOnly(std::vector<uint8_t>& changed_rows);

    wxGrid* m_grid = nullptr;
	wxStaticBoxSizer* static_box = nullptr;
	ModbusItemType& m_items;
	ModbusDataEditDialog* m_StyleDialog = nullptr;
	std::map<uint16_t, ModbusItem*> grid_to_entry;  /* Helper map for storing an additional ID to CanRxData */
	std::string search_pattern;
private:
	bool m_isReadOnly;


	//void OnSize(wxSizeEvent& evt);

    //wxDECLARE_EVENT_TABLE();
};

class ModbusDataPanel : public wxPanel
{
public:
	ModbusDataPanel(wxWindow* parent);
	void On10MsTimer();

	wxBoxSizer* m_hSizer = nullptr;

	ModbusDataEditDialog* m_StyleEditDialog = nullptr;

	ModbusItemPanel* m_coil = nullptr;
	ModbusItemPanel* m_input = nullptr;
	ModbusItemPanel* m_holding = nullptr;
	ModbusItemPanel* m_inputReg = nullptr;

private:
	void OnCellValueChanged(wxGridEvent& ev);
	void OnCellRightClick(wxGridEvent& ev);
	void OnGridLabelLeftClick(wxGridEvent& ev);
	void OnGridLabelRightClick(wxGridEvent& ev);
	void OnKeyDown(wxKeyEvent& evt);

	void OnSize(wxSizeEvent& event);

	wxButton* m_SaveButton = nullptr;
	wxButton* m_StartButton = nullptr;
	wxButton* m_StopButton = nullptr;
	wxSpinCtrl* m_PollingRate = nullptr;
	wxSpinCtrl* m_ResponseTimeout = nullptr;
	wxTextCtrl* m_TcpIp = nullptr;
	wxSpinCtrl* m_TcpPort = nullptr;
	wxSpinCtrl* m_ComPort = nullptr;
	wxCheckBox* m_UseTcp = nullptr;
	wxStaticText* m_ConnectionStatus = nullptr;
	wxTipWindow* tip = nullptr;

	wxDECLARE_EVENT_TABLE();
};

class ModbusLogPanel : public wxPanel, public IModbusHelper
{
public:
	ModbusLogPanel(wxWindow* parent);
	~ModbusLogPanel() = default;

	void AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, uint8_t error, const std::vector<uint8_t>& data) override;
	void ClearRecordingsFromGrid();
	void On10MsTimer();
	void OnKeyDown(wxKeyEvent& evt);

	wxListBox* m_DataLog = nullptr;
	wxStaticBoxSizer* static_box = nullptr;
	wxGrid* m_grid = nullptr;

private:
	void OnSize(wxSizeEvent& event);

	wxButton* m_RecordingStart = nullptr;
	wxButton* m_RecordingPause = nullptr;
	wxButton* m_RecordingStop = nullptr;
	wxButton* m_RecordingClear = nullptr;
	wxButton* m_AutoScrollBtn = nullptr;
	wxButton* m_RecordingSave = nullptr;

	bool m_AutoScroll = false;
	size_t cnt = 0;
	bool is_something_inserted = false;
	std::size_t inserted_until = 0;

	wxDECLARE_EVENT_TABLE();
};

class ModbusMasterPanel : public wxPanel
{
public:
	ModbusMasterPanel(wxWindow* parent);
	~ModbusMasterPanel();

	void UpdateSubpanels();
	void On10MsTimer();

	wxAuiNotebook* m_notebook = nullptr;
	ModbusDataPanel* data_panel = nullptr;
	ModbusLogPanel* log_panel = nullptr;

private:
	void OnSize(wxSizeEvent& evt);
	void Changeing(wxAuiNotebookEvent& event);

	wxAuiManager m_mgr;

	wxDECLARE_EVENT_TABLE();
};