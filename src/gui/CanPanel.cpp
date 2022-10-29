#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanPanel, wxPanel)
EVT_SIZE(CanPanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(CanSenderPanel, wxPanel)
EVT_GRID_CELL_CHANGED(CanSenderPanel::OnCellValueChanged)
EVT_CHAR_HOOK(CanSenderPanel::OnKeyDown)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(CanLogPanel, wxPanel)
EVT_CHAR_HOOK(CanLogPanel::OnKeyDown)
wxEND_EVENT_TABLE()

CanGrid::CanGrid(wxWindow* parent)
{
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(800, 250), 0);

    // Grid
    m_grid->CreateGrid(1, CanSenderGridCol::Sender_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Id, "ID");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_DataSize, "Size");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Data, "Data");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Period, "Period");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Count, "Count");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Comment, "Comment");

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
   

    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->HideRowLabels();
    /*
    m_grid->SetColSize(CanSenderGridCol::Sender_Id, 60);
    m_grid->SetColSize(CanSenderGridCol::Sender_DataSize, 30);*/
    m_grid->SetColSize(CanSenderGridCol::Sender_Data, 200);
    m_grid->SetColSize(CanSenderGridCol::Sender_Comment, 200);
}

void CanGrid::AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Id), id);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_DataSize), dlc);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Data), data);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Period), period);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Count), count);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Comment), comment);

    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_Id, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_Period, new wxGridCellNumberEditor);
    cnt++;
}

void CanGrid::AddRow(std::unique_ptr<CanTxEntry>& e)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Id), wxString::Format("%X", e->id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_DataSize), wxString::Format("%lld", e->data.size()));

    std::string hex;
    utils::ConvertHexBufferToString(e->data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Data), hex);

    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Period), wxString::Format("%d", e->period));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Count), "0");
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Comment), e->comment);

    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_Period, new wxGridCellNumberEditor);

    m_grid->SetReadOnly(cnt, CanSenderGridCol::Sender_Count, true);
    
    for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
        m_grid->SetCellBackgroundColour(cnt, i, (cnt & 1) ? 0xE6E6E6 : 0xFFFFFF);

    grid_to_entry[cnt] = e.get();
    cnt++;
}

void CanGrid::RemoveLastRow()
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows < 1)
        return;
    m_grid->DeleteRows(m_grid->GetNumberRows() - 1, 1);
    cnt--;
    grid_to_entry.erase(cnt);
}

void CanGrid::UpdateTxCounter(uint32_t frame_id, size_t count)
{
    for(auto& i : grid_to_entry)
    {
        if(i.second->id == frame_id)
        {
            int max_rows = m_grid->GetNumberRows();
            if(i.first < max_rows)
                m_grid->SetCellValue(wxGridCellCoords(i.first, CanSenderGridCol::Sender_Count), wxString::Format("%lld", count));
            else
                DBG("invalid column");
            break;
        }
    }
}

CanGridRx::CanGridRx(wxWindow* parent)
{
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(800, 250), 0);

    // Grid
    m_grid->CreateGrid(1, CanSenderGridCol::Sender_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Id, "ID");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_DataSize, "Size");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Data, "Data");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Period, "Period");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Count, "Count");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_Comment, "Comment");

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);


    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->HideRowLabels();

    m_grid->SetColSize(CanSenderGridCol::Sender_Data, 200);
    m_grid->SetColSize(CanSenderGridCol::Sender_Comment, 200);
}

void CanGridRx::AddRow(std::unique_ptr<CanRxData>& e)
{
    m_grid->AppendRows(1);
    int num_row = m_grid->GetNumberRows() - 1;
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Period), "0");
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Count), "1");
    rx_grid_to_entry[num_row] = e.get();

    for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
        m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);

    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Id);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_DataSize);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Data);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Period);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Count);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Id);
    m_grid->SetReadOnly(num_row, CanSenderGridCol::Sender_Id);
}

void CanGridRx::UpdateRow(int num_row, uint32_t frame_id, std::unique_ptr<CanRxData>& e, std::string& comment)
{
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Id), wxString::Format("%X", frame_id));
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_DataSize), wxString::Format("%lld", e->data.size()));

    std::string hex;
    utils::ConvertHexBufferToString(e->data, hex);
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Data), hex);
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Period), wxString::Format("%d", e->period));
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Count), wxString::Format("%lld", e->count));
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Comment), comment);
}

CanSenderPanel::CanSenderPanel(wxWindow* parent) 
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    {
        static_box_rx = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Receive");
        static_box_rx->GetStaticBox()->SetFont(static_box_rx->GetStaticBox()->GetFont().Bold());
        static_box_rx->GetStaticBox()->SetForegroundColour(*wxBLUE);

        can_grid_rx = new CanGridRx(this);

        can_grid_rx->m_grid->DeleteRows(0, can_grid_rx->m_grid->GetNumberRows());
        can_grid_rx->cnt = 0;

        static_box_rx->Add(can_grid_rx->m_grid, 0, wxALL, 5);
        bSizer1->Add(static_box_rx, wxSizerFlags(0).Top());
    }

    {
        static_box_tx = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Transmit");
        static_box_tx->GetStaticBox()->SetFont(static_box_tx->GetStaticBox()->GetFont().Bold());
        static_box_tx->GetStaticBox()->SetForegroundColour(*wxBLUE);

        can_grid_tx = new CanGrid(this);
        RefreshTx();

        Bind(wxEVT_CHAR_HOOK, &CanSenderPanel::OnKeyDown, this);

        static_box_tx->Add(can_grid_tx->m_grid, 0, wxALL, 5);
        bSizer1->Add(static_box_tx, 0, wxALL, 5);

        wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
        m_SingleShot = new wxButton(this, wxID_ANY, "One Shot", wxDefaultPosition, wxDefaultSize);
        m_SingleShot->SetToolTip("Single shot mode for selected CAN frame");
        m_SingleShot->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    entry->single_shot = true;
                    //m_grid->SetCellBackgroundColour(i, 0, *wxGREEN);
                }
            });
        h_sizer->Add(m_SingleShot);


        m_SendAll = new wxButton(this, wxID_ANY, "Send All", wxDefaultPosition, wxDefaultSize);
        m_SendAll->SetToolTip("Send all CAN frame");
        m_SendAll->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                for(auto& i : can_grid_tx->grid_to_entry)
                {
                    i.second->single_shot = false;
                    i.second->send = true;
                }
            });
        h_sizer->Add(m_SendAll);

        m_StopAll = new wxButton(this, wxID_ANY, "Stop All", wxDefaultPosition, wxDefaultSize);
        m_StopAll->SetToolTip("Stop all CAN frame");
        m_StopAll->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                for(auto& i : can_grid_tx->grid_to_entry)
                {
                    i.second->single_shot = false;
                    i.second->send = false;
                }
            });
        h_sizer->Add(m_StopAll);

        m_Add = new wxButton(this, wxID_ANY, "Add", wxDefaultPosition, wxDefaultSize);
        m_Add->SetToolTip("Add CAN frame to the end TX list");
        m_Add->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                std::unique_ptr<CanTxEntry> entry = std::make_unique<CanTxEntry>();
                entry->data = { 0, 0, 0, 0, 0, 0, 0, 0 };
                entry->id = 0x123;
                can_grid_tx->AddRow(entry);
                {
                    std::scoped_lock lock{ can_handler->m };
                    can_handler->entries.push_back(std::move(entry));
                }
            });
        h_sizer->Add(m_Add);

        m_Copy = new wxButton(this, wxID_ANY, "Copy", wxDefaultPosition, wxDefaultSize);
        m_Copy->SetToolTip("Copy CAN frame to the end of TX list");
        m_Copy->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    const CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    std::unique_ptr<CanTxEntry> new_entry = std::make_unique<CanTxEntry>(*entry);
                    can_grid_tx->AddRow(new_entry);
                    {
                        std::scoped_lock lock{ can_handler->m };
                        can_handler->entries.push_back(std::move(new_entry));
                    }
                }
            });
        h_sizer->Add(m_Copy);

        m_Delete = new wxButton(this, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
        m_Delete->SetToolTip("Delete CAN frame from the END of TX list");
        m_Delete->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                can_grid_tx->RemoveLastRow();
                {
                    std::scoped_lock lock{ can_handler->m };
                    can_handler->entries.pop_back();
                }
            });
        h_sizer->Add(m_Delete);

        RefreshGuiIconsBasedOnSettings();

        bSizer1->Add(h_sizer);
    }

    SetSizer(bSizer1);
    Show();
}

CanLogPanel::CanLogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    start_time = std::chrono::steady_clock::now();
    
    CanEntryHandler* can_handler = wxGetApp().can_entry;

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Log :: TX: 0, RX: 0, Total: 0");
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(*wxBLUE);

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(800, 400), 0);

    // Grid
    m_grid->CreateGrid(1, CanLogGridCol::Log_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(CanLogGridCol::Log_Time, "Time");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Direction, "Direction");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Id, "Id");
    m_grid->SetColLabelValue(CanLogGridCol::Log_DataSize, "Size");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Data, "Data");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Comment, "Comment");

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);


    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->HideRowLabels();

    m_grid->SetColSize(CanLogGridCol::Log_Data, 200);
    m_grid->SetColSize(CanLogGridCol::Log_Comment, 200);
    static_box->Add(m_grid);
    
    m_RecordingStart = new wxButton(this, wxID_ANY, "Record", wxDefaultPosition, wxDefaultSize);
    m_RecordingStart->SetToolTip("Start recording for received & sent CAN frames");
    m_RecordingStart->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            CanEntryHandler* can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(true, false);
        });
    h_sizer->Add(m_RecordingStart);

    m_RecordingPause = new wxButton(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize);
    m_RecordingPause->SetToolTip("Suspend recording for received & sent CAN frames");
    m_RecordingPause->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            CanEntryHandler* can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, true);
        });
    h_sizer->Add(m_RecordingPause);

    m_RecordingStop = new wxButton(this, wxID_ANY, "Stop", wxDefaultPosition, wxDefaultSize);
    m_RecordingStop->SetToolTip("Suspend recording for received & sent CAN frames, clear everything");
    m_RecordingStop->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            CanEntryHandler* can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, false);
            inserted_until = 0;
        });
    h_sizer->Add(m_RecordingStop);

    m_RecordingClear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_RecordingClear->SetToolTip("Clear recording");
    m_RecordingClear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            ClearRecordingsFromGrid();

            CanEntryHandler* can_handler = wxGetApp().can_entry;
            can_handler->ClearRecording();
        });
    h_sizer->Add(m_RecordingClear);

    m_RecordingSave = new wxButton(this, wxID_ANY, "Save log", wxDefaultPosition, wxDefaultSize);
    m_RecordingSave->SetToolTip("Save recording");
    m_RecordingSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
#ifdef _WIN32
            const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

            std::string log_format = std::format("CanLog_{:%Y.%m.%d_%H_%M_%OS}.csv", now);
            std::filesystem::path p(log_format);

            CanEntryHandler* can_handler = wxGetApp().can_entry;
            can_handler->SaveRecordingToFile(p);
#endif
        });

    v_sizer->Add(h_sizer);
    v_sizer->Add(m_RecordingSave);
    v_sizer->Add(static_box);

    SetSizerAndFit(v_sizer);
    Show();
}

void CanLogPanel::On10MsTimer()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };

    if(search_pattern.empty())
    {
        static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: TX: %lld, RX: %lld, Total: %lld", can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(),
            can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
    }
    else
    {
        static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: Filter: %s, TX: %lld, RX: %lld, Total: %lld", search_pattern, can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(),
            can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
    }

    if(!can_handler->m_LogEntries.empty())
    {
        if(!is_something_inserted)
            inserted_until = 0;

        auto it = can_handler->m_LogEntries.begin();
        std::advance(it, inserted_until);
        if(it != can_handler->m_LogEntries.end())
        {
            for( ; it != can_handler->m_LogEntries.end(); ++it)
            {
                std::string comment;
                if((*it)->direction == 1)
                {
                    auto comment_it = can_handler->rx_entry_comment.find((*it)->frame_id);
                    if(comment_it != can_handler->rx_entry_comment.end())
                    {
                        comment = comment_it->second;
                    }
                }
                else
                {
                    for(auto& i : can_handler->entries)
                    {
                        if(i->id == (*it)->frame_id)
                        {
                            comment = i->comment;
                            break;
                        }
                    }
                }

                bool insert_row = false;
                if(search_pattern.empty())
                {
                    insert_row = true;
                }
                else
                {
                    if(boost::icontains(comment, search_pattern))
                        insert_row = true;
                }

                if(insert_row)
                    InsertRow((*it)->last_execution, (*it)->direction, (*it)->frame_id, (*it)->data, comment);
            }
            inserted_until = std::distance(can_handler->m_LogEntries.begin(), it);
            is_something_inserted = true;
        }
    }
}

void CanLogPanel::InsertRow(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint32_t id, std::vector<uint8_t>& data, std::string& comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - start_time).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Data), hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Direction), direction == CAN_LOG_DIR_TX ? "TX" : "RX");
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Id), wxString::Format("%X", id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_DataSize), wxString::Format("%lld", data.size()));
    //m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Data), wxString::Format("%s", data));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Comment), comment);

    for(uint8_t i = 0; i != CanLogGridCol::Log_Max; i++)
    {
        m_grid->SetReadOnly(cnt, i, true);
        m_grid->SetCellBackgroundColour(cnt, i, (direction == CAN_LOG_DIR_RX) ? 0xE6E6E6 : 0xFFFFFF);
    }

    cnt++;
}

void CanLogPanel::ClearRecordingsFromGrid()
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows)
        m_grid->DeleteRows(0, num_rows);
    cnt = 0;

    is_something_inserted = false;
    inserted_until = 0;
}

CanPanel::CanPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{

    wxSize client_size = GetClientSize();

    m_mgr.SetManagedWindow(this);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
    sender = new CanSenderPanel(this);
    log = new CanLogPanel(this);
    m_notebook->Freeze();
    m_notebook->AddPage(sender, "Sender", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(log, "Log", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(CanPanel::Changeing), NULL, this);
    m_notebook->Thaw();
}

CanPanel::~CanPanel()
{
    m_mgr.UnInit();  /* deinitialize the frame manager */
}

void CanPanel::RefreshSubpanels()
{
    sender->RefreshSubpanels();
}

void CanPanel::LoadTxList()
{
    sender->LoadTxList();
}

void CanPanel::SaveTxList()
{
    sender->SaveTxList();
}

void CanPanel::LoadRxList()
{
    sender->LoadRxList();
}

void CanPanel::SaveRxList()
{
    sender->SaveRxList();
}

void CanPanel::On10MsTimer()
{
    sender->On10MsTimer();
    log->On10MsTimer();
}

void CanSenderPanel::On10MsTimer()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };

    if(search_pattern_rx.empty())
    {
        for(auto& entry : can_handler->m_rxData)
        {
            bool found_in_grid = false;
            for(auto& i : can_grid_rx->rx_grid_to_entry)
            {
                if(can_grid_rx->rx_grid_to_entry[i.first] == entry.second.get())
                {
                    std::string comment;
                    auto it = can_handler->rx_entry_comment.find(entry.first);
                    if(it != can_handler->rx_entry_comment.end())
                        comment = it->second;
                    can_grid_rx->UpdateRow(i.first, entry.first, entry.second, comment);

                    found_in_grid = true;
                    break;
                }
            }

            if(!found_in_grid)
            {
                can_grid_rx->AddRow(entry.second);
            }
        }
    }
    else
    {
        for(auto& entry : can_handler->m_rxData)
        {
            std::string comment;
            auto it = can_handler->rx_entry_comment.find(entry.first);
            if(it != can_handler->rx_entry_comment.end())
                comment = it->second;
            if(boost::icontains(comment, search_pattern_rx))
            {
                bool found_in_grid = false;
                for(auto& i : can_grid_rx->rx_grid_to_entry)
                {
                    if(can_grid_rx->rx_grid_to_entry[i.first] == entry.second.get())
                    {
                        can_grid_rx->UpdateRow(i.first, entry.first, entry.second, comment);
                        found_in_grid = true;
                        break;
                    }
                }

                if(!found_in_grid)
                {
                    can_grid_rx->AddRow(entry.second);
                }
            }
        }
    }
}

void CanSenderPanel::RefreshSubpanels()
{
    RefreshTx();
    RefreshRx();
    RefreshGuiIconsBasedOnSettings();
}

void CanSenderPanel::RefreshTx()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };
    if(can_grid_tx->m_grid->GetNumberRows())
        can_grid_tx->m_grid->DeleteRows(0, can_grid_tx->m_grid->GetNumberRows());
    can_grid_tx->cnt = 0;
    can_grid_tx->grid_to_entry.clear();

    if(search_pattern_tx.empty())
    {
        for(auto& i : can_handler->entries)
            can_grid_tx->AddRow(i);
    }
    else
    {
        for(auto& i : can_handler->entries)
        {
            if(boost::icontains(i->comment, search_pattern_tx))
                can_grid_tx->AddRow(i);
        }
    }
}

void CanSenderPanel::RefreshRx()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };

    for(int i = 0; i != can_grid_rx->m_grid->GetNumberRows(); i++)
    {
        uint16_t frame_id = std::stoi(can_grid_rx->m_grid->GetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
        auto it = can_handler->rx_entry_comment.find(frame_id);
        can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Comment), it != can_handler->rx_entry_comment.end() ? it->second : "");
    }
}

void CanSenderPanel::RefreshGuiIconsBasedOnSettings()
{
    m_SingleShot->Enable(CanSerialPort::Get()->IsEnabled());
    m_SendAll->Enable(CanSerialPort::Get()->IsEnabled());
    m_StopAll->Enable(CanSerialPort::Get()->IsEnabled());
}

void CanSenderPanel::OnCellValueChanged(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_rx->m_grid))
    {
        wxString new_value = can_grid_rx->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case CanSenderGridCol::Sender_Comment:
            {
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                CanEntryHandler* can_handler = wxGetApp().can_entry;
                std::scoped_lock lock{ can_handler->m };
                can_handler->rx_entry_comment[frame_id] = std::move(new_value.ToStdString());
                break;
            }
        }
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
    {
        wxString new_value = can_grid_tx->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case CanSenderGridCol::Sender_Id:
            {
                uint32_t frame_id = std::stoi(new_value.ToStdString(), nullptr, 16);
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                for(auto& i : can_handler->entries)
                {
                    if(i->id == frame_id)
                    {
                        wxMessageDialog(this, "Given CAN Fame ID already added to the list!", "Error", wxOK).ShowModal();
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Id), wxString::Format("%X", can_grid_tx->grid_to_entry[row]->id));
                        return;
                    }
                }
                can_grid_tx->grid_to_entry[row]->id = frame_id;
                break;
            }
            case CanSenderGridCol::Sender_DataSize:
            {
                uint32_t new_size = std::stoi(new_value.ToStdString());
                if(new_size > 8)
                {
                    wxMessageDialog(this, "Max payload size is 8!", "Error", wxOK).ShowModal();
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_DataSize), wxString::Format("%lld", can_grid_tx->grid_to_entry[row]->data.size()));
                    return;
                }

                can_grid_tx->grid_to_entry[row]->data.resize(new_size);

                std::string hex;
                utils::ConvertHexBufferToString(can_grid_tx->grid_to_entry[row]->data, hex);
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Data), wxString(hex));
                break;
            }
            case CanSenderGridCol::Sender_Data:
            {
                char bytes[128] = { 0 };
                std::string hex_str = new_value.ToStdString();
                boost::algorithm::erase_all(hex_str, " ");
                if(hex_str.length() > 16)
                    hex_str.erase(16, hex_str.length() - 16);
                utils::ConvertHexStringToBuffer(hex_str, std::span{ bytes });
                can_grid_tx->grid_to_entry[row]->data.assign(bytes, bytes + (hex_str.length() / 2));

                std::string hex;
                utils::ConvertHexBufferToString(can_grid_tx->grid_to_entry[row]->data, hex);
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, col), wxString(hex));
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_DataSize), wxString::Format("%lld", hex_str.length() / 2));
                break;
            }
            case CanSenderGridCol::Sender_Period:
            {
                if(new_value == "off")
                    new_value = "0";
                int period = std::stoi(new_value.ToStdString());
                if(period < 0)
                {
                    wxMessageDialog(this, "Period can't be negative!", "Error", wxOK).ShowModal();
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Period), wxString::Format("%d", can_grid_tx->grid_to_entry[row]->period));
                    return;
                }
                can_grid_tx->grid_to_entry[row]->period = period;
                break;
            }
            case CanSenderGridCol::Sender_Comment:
            {
                can_grid_tx->grid_to_entry[row]->comment = std::move(new_value.ToStdString());
                break;
            }
        }
    }
    ev.Skip();
}

void CanPanel::Changeing(wxAuiNotebookEvent& event)
{
    /*
    int sel = event.GetSelection();
    if(sel == 0)
    {
        comtcp_panel->Update();
    }
    */
}

void CanSenderPanel::LoadTxList()
{
    wxFileDialog openFileDialog(this, _("Open TX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_tx = openFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_tx.ToStdString();
    bool ret = can_handler->LoadTxList(p);
    RefreshTx();

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(ret)
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListLoaded) });
    else
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListLoadError) });
}

void CanSenderPanel::SaveTxList()
{
    wxFileDialog saveFileDialog(this, _("Save TX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_tx = saveFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_tx.ToStdString();
    can_handler->SaveTxList(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListSaved) });
}

void CanSenderPanel::LoadRxList()
{
    wxFileDialog openFileDialog(this, _("Open RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_rx = openFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_rx.ToStdString();
    bool ret = can_handler->LoadRxList(p);
    RefreshRx();

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(ret)
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListLoaded) });
    else
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListLoadError) });
}
 
void CanSenderPanel::SaveRxList()
{
    wxFileDialog saveFileDialog(this, _("Save RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_rx = saveFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_rx.ToStdString();
    can_handler->SaveRxList(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListSaved) });
}

void CanPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CanSenderPanel::OnKeyDown(wxKeyEvent& evt)
{
    if(evt.ControlDown())
    {
        switch(evt.GetKeyCode())
        {
            case 'F':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == can_grid_tx->m_grid)
                {
                    wxTextEntryDialog d(this, "Enter TX frame name for what you want to filter", "Search for frame");
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {
                        search_pattern_tx = d.GetValue().ToStdString();
                        if(search_pattern_tx.empty())
                            static_box_tx->GetStaticBox()->SetLabelText("Transmit");
                        else
                            static_box_tx->GetStaticBox()->SetLabelText(wxString::Format("Transmit - Search filter: %s", search_pattern_tx));

                        RefreshTx();
                    }
                }
                else if(focus == can_grid_rx->m_grid)
                {
                    wxTextEntryDialog d(this, "Enter RX frame name for what you want to filter", "Search for frame");
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {
                        search_pattern_rx = d.GetValue().ToStdString();
                        if(search_pattern_rx.empty())
                            static_box_rx->GetStaticBox()->SetLabelText("Receive");
                        else
                            static_box_rx->GetStaticBox()->SetLabelText(wxString::Format("Receive - Search filter: %s", search_pattern_rx));

                        if(can_grid_rx->m_grid->GetNumberRows())
                            can_grid_rx->m_grid->DeleteRows(0, can_grid_rx->m_grid->GetNumberRows());
                        can_grid_rx->cnt = 0;
                        can_grid_rx->rx_grid_to_entry.clear();
                        RefreshRx();
                    }
                }
                break;
            }
        }
    }
    evt.Skip();
}

void CanLogPanel::OnKeyDown(wxKeyEvent& evt)
{
    if(evt.ControlDown())
    {
        switch(evt.GetKeyCode())
        {
            case 'F':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == m_grid)
                {
                    wxTextEntryDialog d(this, "Enter frame name for what you want to filter", "Frame filter");
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {
                        std::string new_search_pattern = d.GetValue().ToStdString();
                        if(new_search_pattern != search_pattern)
                        {
                            search_pattern = new_search_pattern;
                            ClearRecordingsFromGrid();
                        }
                        /*
                        if(search_pattern.empty())
                            //static_box_tx->GetStaticBox()->SetLabelText("Transmit");
                        else
                            //static_box_tx->GetStaticBox()->SetLabelText(wxString::Format("Transmit - Search filter: %s", search_pattern_tx));
                            */
                            //RefreshTx();
                    }
                }
            }
        }
    }
}
