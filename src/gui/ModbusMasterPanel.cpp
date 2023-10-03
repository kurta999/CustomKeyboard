#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ModbusDataPanel, wxPanel)
EVT_GRID_CELL_CHANGED(ModbusDataPanel::OnCellValueChanged)
EVT_SIZE(ModbusDataPanel::OnSize)
EVT_GRID_CELL_RIGHT_CLICK(ModbusDataPanel::OnCellRightClick)
EVT_GRID_LABEL_RIGHT_CLICK(ModbusDataPanel::OnGridLabelRightClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusLogPanel, wxPanel)
EVT_SIZE(ModbusLogPanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusMasterPanel, wxPanel)
EVT_SIZE(ModbusMasterPanel::OnSize)
wxEND_EVENT_TABLE()

ModbusItemPanel::ModbusItemPanel(wxWindow* parent, const wxString& header_name, ModbusItemType& items, bool is_read_only)
    : m_items(items), m_isReadOnly(is_read_only)
{
	static_box = new wxStaticBoxSizer(wxVERTICAL, parent, header_name);
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(wxColor(235, 52, 204));

    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 700), 0);

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

void ModbusItemPanel::UpdatePanel()
{
    if(m_grid->GetNumberRows())
        m_grid->DeleteRows(0, m_grid->GetNumberRows());

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    for(auto& i : m_items)
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

void ModbusItemPanel::UpdateChangesOnly(std::vector<uint8_t>& changed_rows)
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

        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), m_items[i]->m_Name);
        m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%lld", m_items[i]->m_Value));

        for(uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);
        m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, m_isReadOnly);
    }
}


ModbusDataPanel::ModbusDataPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    m_coil = new ModbusItemPanel(this, "Coil Status", modbus_handler->m_coils, false);
    m_input = new ModbusItemPanel(this, "Input Status", modbus_handler->m_inputStatus, true);
    m_holding = new ModbusItemPanel(this, "Holding Registers", modbus_handler->m_Holding, false);
    m_inputReg = new ModbusItemPanel(this, "Input Registers", modbus_handler->m_Input, true);

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);

    m_hSizer = new wxBoxSizer(wxHORIZONTAL);
    m_hSizer->Add(m_coil->static_box);
    m_hSizer->Add(m_input->static_box);
    m_hSizer->Add(m_holding->static_box);
    m_hSizer->Add(m_inputReg->static_box);

    v_sizer->Add(m_hSizer);

    wxBoxSizer* h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    m_StartButton = new wxButton(this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    m_StartButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->SetPollingStatus(true);
        });
    h_sizer2->Add(m_StartButton);

    m_StopButton = new wxButton(this, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    m_StopButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->SetPollingStatus(false);

        });
    h_sizer2->Add(m_StopButton);

    m_SaveButton = new wxButton(this, wxID_ANY, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0);
    m_SaveButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->Save();
        });
    h_sizer2->Add(m_SaveButton);

    h_sizer2->Add(new wxStaticText(this, wxID_ANY, "Polling rate [ms]: "));
    h_sizer2->AddSpacer(5);
    m_PollingRate = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 50, 10000, modbus_handler->GetPollingRate());
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

void ModbusDataPanel::OnCellValueChanged(wxGridEvent& ev)
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
                m_coil->m_items[row]->m_Name = new_value.ToStdString();
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
                m_input->m_items[row]->m_Name = new_value.ToStdString();
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
                m_holding->m_items[row]->m_Name = new_value.ToStdString();
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

void ModbusDataPanel::OnCellRightClick(wxGridEvent& ev)
{

}

void ModbusDataPanel::OnGridLabelRightClick(wxGridEvent& ev)
{

}

void ModbusDataPanel::OnSize(wxSizeEvent& event)
{
    event.Skip(true);
}

ModbusLogPanel::ModbusLogPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    modbus_handler->SetModbusHelper(this);

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_RecordingStart = new wxButton(this, wxID_ANY, "Record", wxDefaultPosition, wxDefaultSize);
    m_RecordingStart->SetToolTip("Start recording for received & sent Modbus frames");
    m_RecordingStart->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->ToggleRecording(true, false);
        });
    h_sizer->Add(m_RecordingStart);

    m_RecordingPause = new wxButton(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize);
    m_RecordingPause->SetToolTip("Suspend recording for received & sent Modbus frames");
    m_RecordingPause->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->ToggleRecording(false, true);
        });
    h_sizer->Add(m_RecordingPause);

    m_RecordingStop = new wxButton(this, wxID_ANY, "Stop", wxDefaultPosition, wxDefaultSize);
    m_RecordingStop->SetToolTip("Suspend recording for received & sent Modbus frames, clear everything");
    m_RecordingStop->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>&modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->ToggleRecording(false, false);
            inserted_until = 0;
        });
    h_sizer->Add(m_RecordingStop);

    m_RecordingClear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_RecordingClear->SetToolTip("Clear recording and reset frame counters");
    m_RecordingClear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            ClearRecordingsFromGrid();

            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->ClearRecording();
        });
    h_sizer->Add(m_RecordingClear);

    m_AutoScrollBtn = new wxButton(this, wxID_ANY, wxT("Toggle auto-scroll"), wxDefaultPosition, wxDefaultSize, 0);
    m_AutoScrollBtn->SetToolTip("Toggle auto-scroll");
    m_AutoScrollBtn->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            m_AutoScroll ^= 1;
            if(m_AutoScroll)
                m_AutoScrollBtn->SetBackgroundColour(wxNullColour);
            else
                m_AutoScrollBtn->SetBackgroundColour(*wxRED);
        });
    h_sizer->AddSpacer(35);
    h_sizer->Add(m_AutoScrollBtn);

    v_sizer->Add(h_sizer);

    wxBoxSizer* h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    m_RecordingSave = new wxButton(this, wxID_ANY, "Save log", wxDefaultPosition, wxDefaultSize);
    m_RecordingSave->SetToolTip("Save recording to file");
    m_RecordingSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
#ifdef _WIN32
            const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

            if(std::filesystem::exists("Modbus"))
                std::filesystem::create_directory("Modbus");
            std::string log_format = std::format("Modbus/ModbusLog_{:%Y.%m.%d_%H_%M_%OS}.csv", now);
            std::filesystem::path p(log_format);

            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->SaveRecordingToFile(p);
#endif
        });
    h_sizer2->Add(m_RecordingSave);

    v_sizer->Add(h_sizer2);

    static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Log :: TX: 0, RX: 0, Err: 0 - Total: 0");
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(*wxBLUE);

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600), 0);

    // Grid
    m_grid->CreateGrid(1, ModbusLogGridCol::ModbusLog_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_Time, "Time");
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_Direction, "Dir");
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_FCode, "FC");
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_DataSize, "Size");
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_Data, "Data");

    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Time, 50);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Direction, 50);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_FCode, 50);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_DataSize, 50);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Data, 500);

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    m_grid->SetRowLabelSize(40);

    static_box->Add(m_grid);
    v_sizer->Add(static_box, wxSizerFlags(1).Top().Expand());

    SetSizer(v_sizer);
    Show();
}

void ModbusLogPanel::AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, const std::vector<uint8_t>& data)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - modbus_handler->GetStartTime()).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Data), hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Direction), direction == CAN_LOG_DIR_TX ? "TX" : "RX");
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_FCode), wxString::Format("%X", fcode));
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_DataSize), wxString::Format("%lld", data.size()));

    if(m_AutoScroll)
        m_grid->ScrollLines(num_rows);

    for(uint8_t i = 0; i != ModbusLogGridCol::ModbusLog_Max; i++)
    {
        m_grid->SetReadOnly(cnt, i, true);
        m_grid->SetCellBackgroundColour(cnt, i, (direction == CAN_LOG_DIR_RX) ? 0xE6E6E6 : 0xFFFFFF);
    }

    cnt++;
}

void ModbusLogPanel::ClearRecordingsFromGrid()
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows)
        m_grid->DeleteRows(0, num_rows);
    cnt = 0;

    is_something_inserted = false;
    inserted_until = 0;
}

void ModbusLogPanel::On10MsTimer()
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    static uint64_t last_tx_cnt = 0, last_rx_cnt = 0, last_err_cnt = 0;
    if(last_tx_cnt != modbus_handler->GetTxFrameCount() || last_rx_cnt != modbus_handler->GetRxFrameCount() || last_err_cnt != modbus_handler->GetErrFrameCount())
    {
        static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: TX: %lld, RX: %lld, ERR: %lld, Total: %lld", modbus_handler->GetTxFrameCount(), modbus_handler->GetRxFrameCount(),
            modbus_handler->GetErrFrameCount(), modbus_handler->GetTxFrameCount() + modbus_handler->GetRxFrameCount()));
    }

    last_tx_cnt = modbus_handler->GetTxFrameCount();
    last_rx_cnt = modbus_handler->GetRxFrameCount();
    last_err_cnt = modbus_handler->GetErrFrameCount();

    if(!modbus_handler->m_LogEntries.empty())
    {
        if(!is_something_inserted)
            inserted_until = 0;

        auto it = modbus_handler->m_LogEntries.begin();
        std::advance(it, inserted_until);
        if(it == modbus_handler->m_LogEntries.end())
        {
            //DBG("shit happend");
            return;
        }


        if(it != modbus_handler->m_LogEntries.end())
        {
            for(; it != modbus_handler->m_LogEntries.end(); ++it)
            {
                AppendLog((*it)->last_execution, (*it)->direction, (*it)->fcode, (*it)->data);
                inserted_until++;
            }
            //inserted_until = std::distance(can_handler->m_LogEntries.begin(), it);
            is_something_inserted = true;
        }
    }
}

void ModbusLogPanel::OnSize(wxSizeEvent& event)
{
    event.Skip(true);
}

ModbusMasterPanel::ModbusMasterPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
    wxSize client_size = GetClientSize();

	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    data_panel = new ModbusDataPanel(this);
    log_panel = new ModbusLogPanel(this);

    m_notebook->Freeze();
    m_notebook->AddPage(data_panel, "Data", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(log_panel, "Log", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ModbusMasterPanel::Changeing), NULL, this);
    m_notebook->Split(0, wxLEFT);
    m_notebook->Thaw();
}

void ModbusMasterPanel::UpdateSubpanels()
{

}

void ModbusMasterPanel::On10MsTimer()
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    std::scoped_lock lock{ modbus_handler->m };

    if(log_panel)
        log_panel->On10MsTimer();
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