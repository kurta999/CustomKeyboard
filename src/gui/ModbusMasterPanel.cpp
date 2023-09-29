#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ModbusMasterPanel, wxPanel)
EVT_SIZE(ModbusMasterPanel::OnSize)
EVT_GRID_CELL_CHANGED(ModbusMasterPanel::OnCellValueChanged)
wxEND_EVENT_TABLE()
/*
wxBEGIN_EVENT_TABLE(CoilStatusPanel, wxPanel)
EVT_SIZE(CoilStatusPanel::OnSize)
wxEND_EVENT_TABLE()
*/
CoilStatusPanel::CoilStatusPanel(wxWindow* parent, const wxString& header_name, ModbusCoils& coils, bool is_read_only)/* :
    wxPanel(parent, wxID_ANY)*/
    : m_coils(coils), m_isReadOnly(is_read_only)
{
	static_box = new wxStaticBoxSizer(wxVERTICAL, parent, header_name);
	
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 800), 0);

    // Grid
    m_grid->CreateGrid(0, ModbusGridCol::Modbus_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Name, "Name");
    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Value, "Value");
    
    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);

    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectCells);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);


    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->SetRowLabelSize(30);

    //m_grid->HideRowLabels();

    static_box->Add(m_grid);

    UpdatePanel();
}

void CoilStatusPanel::UpdatePanel()
{
    if(m_grid->GetNumberRows())
        m_grid->DeleteRows(0, m_grid->GetNumberRows());

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    for(auto& i : m_coils)
    {
        m_grid->AppendRows(1);
        int num_row = m_grid->GetNumberRows() - 1;
        m_grid->SetRowLabelValue(num_row, wxString::Format("%d", num_row));

        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), i.first);
        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%d", i.second));

        for(uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
        {
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);

        }
        if(m_isReadOnly)
            m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, true);
    }
}

void CoilStatusPanel::UpdateChangesOnly(std::vector<uint8_t>& changed_rows)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    for(auto& i : changed_rows)
    {
        int num_row = m_grid->GetNumberRows();
        if(i >= num_row)
            m_grid->AppendRows(1);
        else
            num_row = i;

        m_grid->SetRowLabelValue(num_row, wxString::Format("%d", num_row));

        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), m_coils[i].first);
        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%d", m_coils[i].second));

        for(uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);
        m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, m_isReadOnly);
    }
}



ModbusRegisterPanel::ModbusRegisterPanel(wxWindow* parent, const wxString& header_name, ModbusHoldingRegisters& holding, bool is_read_only)
    : m_holding(holding), m_isReadOnly(is_read_only)
{
    static_box = new wxStaticBoxSizer(wxVERTICAL, parent, header_name);

    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 800), 0);

    // Grid
    m_grid->CreateGrid(0, ModbusGridCol::Modbus_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Name, "Name");
    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Value, "Value");

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);

    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectCells);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);


    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->SetRowLabelSize(30);

    //m_grid->HideRowLabels();

    static_box->Add(m_grid);

    UpdatePanel();
}

void ModbusRegisterPanel::UpdatePanel()
{
    if(m_grid->GetNumberRows())
        m_grid->DeleteRows(0, m_grid->GetNumberRows());

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    for(auto& i : m_holding)
    {
        m_grid->AppendRows(1);
        int num_row = m_grid->GetNumberRows() - 1;
        m_grid->SetRowLabelValue(num_row, wxString::Format("%d", num_row));

        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), i->m_Name);
        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%lld", i->m_Value));

        for(uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
        {
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);

        }
        if(m_isReadOnly)
            m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, true);
    }
}

void ModbusRegisterPanel::UpdateChangesOnly(std::vector<uint8_t>& changed_rows)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    for(auto& i : changed_rows)
    {
        int num_row = m_grid->GetNumberRows();
        if(i >= num_row)
            m_grid->AppendRows(1);
        else
            num_row = i;

        m_grid->SetRowLabelValue(num_row, wxString::Format("%d", num_row));

        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), m_holding[i]->m_Name);
        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%lld", m_holding[i]->m_Value));

        for(uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);
        m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, m_isReadOnly);
    }
}

void ModbusMasterPanel::OnCellValueChanged(wxGridEvent& ev)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(m_coil->m_grid))
    {
        wxString new_value = m_coil->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case ModbusGridCol::Modbus_Name:
            {
                m_coil->m_coils[row].first = new_value.ToStdString();
                break;
            }
            case ModbusGridCol::Modbus_Value:
            {
                bool is_set = std::stoi(new_value.ToStdString()) != 0;

                modbus_handler->EditCoil(row, is_set);
                break;
            }
        }
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(m_input->m_grid))
    {
        wxString new_value = m_input->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case ModbusGridCol::Modbus_Name:
            {
                m_input->m_coils[row].first = new_value.ToStdString();
                break;
            }
        }
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(m_holding->m_grid))
    {
        wxString new_value = m_holding->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case ModbusGridCol::Modbus_Name:
            {
                m_holding->m_holding[row]->m_Name = new_value.ToStdString();
                break;
            }
            case ModbusGridCol::Modbus_Value:
            {
                uint64_t val = std::stoi(new_value.ToStdString());

                modbus_handler->EditHolding(row, val);
                break;
            }

        }
    }
}

ModbusMasterPanel::ModbusMasterPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
    /*
    wxSize client_size = GetClientSize();

	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    m_coil = new CoilStatusPanel(this);

    m_notebook->Freeze();
    m_notebook->AddPage(m_coil, "Coil Status", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ConfigurationPanel::Changeing), NULL, this);
    m_notebook->Thaw();
    */
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    m_coil = new CoilStatusPanel(this, "Coil status", modbus_handler->m_coils, false);
    m_input = new CoilStatusPanel(this, "Input Status", modbus_handler->m_inputStatus, true);
    m_holding = new ModbusRegisterPanel(this, "Holding registers", modbus_handler->m_Holding, false);
    m_inputReg = new ModbusRegisterPanel(this, "Input registers", modbus_handler->m_Input, true);

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);

    m_hSizer = new wxBoxSizer(wxHORIZONTAL);
    m_hSizer->Add(m_coil->static_box);
    m_hSizer->Add(m_input->static_box);
    m_hSizer->Add(m_holding->static_box);
    m_hSizer->Add(m_inputReg->static_box);

    m_DataLog = new wxListBox(this, wxID_ANY);
    m_hSizer->Add(m_DataLog, wxSizerFlags(1).Top().Expand());
    v_sizer->Add(m_hSizer);

    wxBoxSizer* h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    m_SaveButton = new wxButton(this, wxID_ANY, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0);
    m_SaveButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->Save();
        });
    h_sizer2->Add(m_SaveButton);

    h_sizer2->Add(new wxStaticText(this, wxID_ANY, "Polling rate [ms]: "));
    h_sizer2->AddSpacer(5);
    m_PollingRate = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 50, 10000, 500);
    m_PollingRate->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, [this](wxSpinEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            int new_value = event.GetValue();
            modbus_handler->SetPollingRate(new_value);
        });

    h_sizer2->Add(m_PollingRate);

    v_sizer->Add(h_sizer2);

    SetSizer(v_sizer);
    Show();
}

void ModbusMasterPanel::UpdateSubpanels()
{

}

void ModbusMasterPanel::OnSize(wxSizeEvent& event)
{
	event.Skip(true);
}

void ModbusMasterPanel::Changeing(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if(sel == 0)
	{
		//comtcp_panel->Update();
	}
}