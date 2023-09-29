#pragma once

#include "IModbusEntry.hpp"

enum ModbusGridCol : int
{
    Modbus_Name,
    Modbus_Value,
	Modbus_Max
};

class CoilStatusPanel/* : public wxPanel*/
{
public:
    CoilStatusPanel(wxWindow* parent, const wxString& header_name, ModbusCoils& coils, bool is_read_only);

    void UpdatePanel();
    void UpdateChangesOnly(std::vector<uint8_t>& changed_rows);

    wxGrid* m_grid = nullptr;
	wxStaticBoxSizer* static_box = nullptr;
	ModbusCoils& m_coils;

private:
	bool m_isReadOnly;


	//void OnSize(wxSizeEvent& evt);

    //wxDECLARE_EVENT_TABLE();
};

class ModbusRegisterPanel/* : public wxPanel*/
{
public:
	ModbusRegisterPanel(wxWindow* parent, const wxString& header_name, ModbusHoldingRegisters& holding, bool is_read_only);

    void UpdatePanel();
    void UpdateChangesOnly(std::vector<uint8_t>& changed_rows);

    wxGrid* m_grid = nullptr;
	wxStaticBoxSizer* static_box = nullptr;
	ModbusHoldingRegisters& m_holding;

private:
	bool m_isReadOnly;


	//void OnSize(wxSizeEvent& evt);

    //wxDECLARE_EVENT_TABLE();
};

class ModbusMasterPanel : public wxPanel
{
public:
	ModbusMasterPanel(wxWindow* parent);
	void UpdateSubpanels();

	wxBoxSizer* m_hSizer = nullptr;
	wxListBox* m_DataLog = nullptr;

	CoilStatusPanel* m_coil = nullptr;
	CoilStatusPanel* m_input = nullptr;
	ModbusRegisterPanel* m_holding = nullptr;
	ModbusRegisterPanel* m_inputReg = nullptr;
	wxAuiNotebook* m_notebook = nullptr;

private:
	void OnSize(wxSizeEvent& evt);
	void OnCellValueChanged(wxGridEvent& ev);

	void Changeing(wxAuiNotebookEvent& event);

	wxButton* m_SaveButton = nullptr;
	wxSpinCtrl* m_PollingRate = nullptr;
	wxDECLARE_EVENT_TABLE();
};