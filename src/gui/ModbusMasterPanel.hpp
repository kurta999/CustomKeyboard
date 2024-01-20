#pragma once

#include "IModbusEntry.hpp"

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
	ModbusLog_DataSize,
	ModbusLog_Data,
	ModbusLog_Max
};


class ModbusItemPanel
{
public:
    ModbusItemPanel(wxWindow* parent, const wxString& header_name, ModbusItemType& items, bool is_read_only);

    void UpdatePanel();
    void UpdateChangesOnly(std::vector<uint8_t>& changed_rows);

    wxGrid* m_grid = nullptr;
	wxStaticBoxSizer* static_box = nullptr;
	ModbusItemType& m_items;

private:
	bool m_isReadOnly;


	//void OnSize(wxSizeEvent& evt);

    //wxDECLARE_EVENT_TABLE();
};

class ModbusDataPanel : public wxPanel
{
public:
	ModbusDataPanel(wxWindow* parent);

	wxBoxSizer* m_hSizer = nullptr;

	ModbusItemPanel* m_coil = nullptr;
	ModbusItemPanel* m_input = nullptr;
	ModbusItemPanel* m_holding = nullptr;
	ModbusItemPanel* m_inputReg = nullptr;

private:
	void OnCellValueChanged(wxGridEvent& ev);
	void OnCellRightClick(wxGridEvent& ev);
	void OnGridLabelRightClick(wxGridEvent& ev);

	void OnSize(wxSizeEvent& event);

	wxButton* m_SaveButton = nullptr;
	wxButton* m_StartButton = nullptr;
	wxButton* m_StopButton = nullptr;
	wxSpinCtrl* m_PollingRate = nullptr;
	wxSpinCtrl* m_ResponseTimeout = nullptr;

	wxDECLARE_EVENT_TABLE();
};

class ModbusLogPanel : public wxPanel, public IModbusHelper
{
public:
	ModbusLogPanel(wxWindow* parent);
	~ModbusLogPanel() = default;

	void AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, const std::vector<uint8_t>& data) override;
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
	void UpdateSubpanels();
	void On10MsTimer();

	wxAuiNotebook* m_notebook = nullptr;
	ModbusDataPanel* data_panel;
	ModbusLogPanel* log_panel;

private:
	void OnSize(wxSizeEvent& evt);

	void Changeing(wxAuiNotebookEvent& event);

	wxDECLARE_EVENT_TABLE();
};