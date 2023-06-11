#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanSenderPanel, wxPanel)
EVT_SIZE(CanSenderPanel::OnSize)
EVT_GRID_CELL_CHANGED(CanSenderPanel::OnCellValueChanged)
/*
EVT_GRID_CELL_LEFT_CLICK(CanSenderPanel::OnCellLeftClick)
EVT_GRID_CELL_LEFT_DCLICK(CanSenderPanel::OnCellLeftDoubleClick)
*/
EVT_GRID_CELL_RIGHT_CLICK(CanSenderPanel::OnCellRightClick)
EVT_GRID_LABEL_RIGHT_CLICK(CanSenderPanel::OnGridLabelRightClick)
EVT_CHAR_HOOK(CanSenderPanel::OnKeyDown)
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
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_LogLevel, "Log");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_FavouriteLevel, "Fav");
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
    m_grid->SetColSize(CanSenderGridCol::Sender_LogLevel, 35);
    m_grid->SetColSize(CanSenderGridCol::Sender_FavouriteLevel, 35);
    m_grid->SetColSize(CanSenderGridCol::Sender_Comment, 160);

    m_grid->GetGridWindow()->Bind(wxEVT_MIDDLE_DOWN, [this](wxMouseEvent& event)
        {
            DBG("middle down\n");
        });
}

void CanGrid::AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString loglevel, wxString comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Id), id);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_DataSize), dlc);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Data), data);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Period), period);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Count), count);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_LogLevel), loglevel);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Comment), comment);

    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_Period, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_LogLevel, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_FavouriteLevel, new wxGridCellNumberEditor);

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
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", e->log_level));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_FavouriteLevel), wxString::Format("%d", e->favourite_level));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanSenderGridCol::Sender_Comment), e->comment);

    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_Period, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_LogLevel, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_FavouriteLevel, new wxGridCellNumberEditor);

    m_grid->SetReadOnly(cnt, CanSenderGridCol::Sender_Count, true);

    if(e->m_color)
    {
        for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
            m_grid->SetCellTextColour(cnt, i, RGB_TO_WXCOLOR(*e->m_color));
    }

    if(e->m_bg_color)  /* Set custom color if it's given */
    {
        for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
            m_grid->SetCellBackgroundColour(cnt, i, RGB_TO_WXCOLOR(*e->m_bg_color));
    }
    else  /* Otherway use two colors alternately for all of the lines */
    {
        for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
            m_grid->SetCellBackgroundColour(cnt, i, (cnt & 1) ? 0xE6E6E6 : 0xFFFFFF);
    }

    if(e->m_is_bold)
    {
        wxFont font;
        font.SetWeight(wxFONTWEIGHT_BOLD);
        for(uint8_t i = 0; i != CanSenderGridCol::Sender_Max; i++)
            m_grid->SetCellFont(cnt, i, font);
    }

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
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_LogLevel, "Log");
    m_grid->SetColLabelValue(CanSenderGridCol::Sender_FavouriteLevel, "Fav");
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
    m_grid->SetColSize(CanSenderGridCol::Sender_LogLevel, 30);
    m_grid->SetColSize(CanSenderGridCol::Sender_FavouriteLevel, 30);
    m_grid->SetColSize(CanSenderGridCol::Sender_Comment, 160);

    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_LogLevel, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, CanSenderGridCol::Sender_FavouriteLevel, new wxGridCellNumberEditor);
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
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", e->log_level));
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_FavouriteLevel), wxString::Format("%d", e->favourite_level));
    m_grid->SetCellValue(wxGridCellCoords(num_row, CanSenderGridCol::Sender_Comment), comment);
}

void CanGridRx::ClearGrid()
{
    rx_grid_to_entry.clear();  /* Clear entrie RX grid */
    cnt = 0;
    if(m_grid->GetNumberRows())
        m_grid->DeleteRows(0, m_grid->GetNumberRows());
}

CanSenderPanel::CanSenderPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    m_BitfieldEditor = new BitEditorDialog(this);
    m_LogForFrame = new CanLogForFrameDialog(this);
    m_UdsRawDialog = new CanUdsRawDialog(this);

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
        m_SingleShot->SetToolTip("Single shot mode for selected CAN frame (or force sending if it's sent periodically)");
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

        m_SendSelected = new wxButton(this, wxID_ANY, "Send selected", wxDefaultPosition, wxDefaultSize);
        m_SendSelected->SetToolTip("Start sending selected CAN frames (which period isn't 0)");
        m_SendSelected->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    entry->single_shot = false;
                    entry->send = true;
                }
            });
        h_sizer->Add(m_SendSelected);

        m_StopSelected = new wxButton(this, wxID_ANY, "Stop selected", wxDefaultPosition, wxDefaultSize);
        m_StopSelected->SetToolTip("Stop sending selected CAN frames (which period isn't 0)");
        m_StopSelected->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    entry->single_shot = false;
                    entry->send = false;
                }
            });
        h_sizer->Add(m_StopSelected);

        m_SendAll = new wxButton(this, wxID_ANY, "Send All", wxDefaultPosition, wxDefaultSize);
        m_SendAll->SetToolTip("Start send all CAN frame (which period isn't 0)");
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
        m_Add->SetToolTip("Add CAN frame below selection");
        m_Add->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty() || rows.size() > 1) return;

                std::unique_ptr<CanTxEntry> entry = std::make_unique<CanTxEntry>();
                entry->data = { 0, 0, 0, 0, 0, 0, 0, 0 };
                entry->id = 0x123;

                while(std::find_if(can_handler->entries.begin(), can_handler->entries.end(),
                    [frame_id = entry->id](const auto& item) { return item->id == frame_id; }) != can_handler->entries.end())  /* Protection against same Frame IDs */
                {
                    entry->id++;
                }

                can_grid_tx->AddRow(entry);
                {
                    std::scoped_lock lock{ can_handler->m };
                    can_handler->entries.insert(can_handler->entries.begin() + (rows[0] + 1), std::move(entry));
                }

                RefreshTx();
                m_grid->SelectRow(rows[0] + 1);
            });
        h_sizer->Add(m_Add);

        m_Copy = new wxButton(this, wxID_ANY, "Copy", wxDefaultPosition, wxDefaultSize);
        m_Copy->SetToolTip("Copy selected CAN frame(s) to the end of TX list");
        m_Copy->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
                m_grid->SelectRow(m_grid->GetNumberRows() - 1);
            });
        h_sizer->Add(m_Copy);

        wxSize ret_size = m_Copy->GetSize();
        m_MoveUp = new wxButton(this, wxID_ANY, "Move Up", wxDefaultPosition, wxDefaultSize);
        //m_MoveUp = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(75, 16))));
        m_MoveUp->SetToolTip("Move Up selected TX entry");
        m_MoveUp->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                int selection = m_grid->GetNumberRows() - 1;
                for(auto& i : rows)
                {
                    uint32_t frame_id = std::stoi(can_grid_tx->m_grid->GetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
                    const CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    {
                        std::scoped_lock lock{ can_handler->m };

                        if(can_handler->entries.front()->id == frame_id)
                        {
                            std::rotate(can_handler->entries.begin(), can_handler->entries.begin() + 1, can_handler->entries.end());
                        }
                        else
                        {
                            uint32_t frame_id_new = std::stoi(can_grid_tx->m_grid->GetCellValue(wxGridCellCoords(i - 1, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
                            auto this_entry = std::find_if(can_handler->entries.begin(), can_handler->entries.end(), [&frame_id](const auto& item) { return item->id == frame_id; });
                            auto new_entry = std::find_if(can_handler->entries.begin(), can_handler->entries.end(), [&frame_id_new](const auto& item) { return item->id == frame_id_new; });

                            std::iter_swap(this_entry, new_entry);
                            selection = std::distance(can_handler->entries.begin(), new_entry);
                        }
                    }
                }

                RefreshTx();
                m_grid->SelectRow(selection);
            });
        h_sizer->Add(m_MoveUp);

        m_MoveDown = new wxButton(this, wxID_ANY, "Move Down", wxDefaultPosition, wxDefaultSize);
        m_MoveDown->SetToolTip("Move Down selected TX entry");
        m_MoveDown->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                int selection = 0;
                for(auto& i : rows)
                {
                    uint32_t frame_id = std::stoi(can_grid_tx->m_grid->GetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
                    const CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    {
                        std::scoped_lock lock{ can_handler->m };

                        if(can_handler->entries.back()->id == frame_id)
                        {
                            std::rotate(can_handler->entries.rbegin(), can_handler->entries.rbegin() + 1, can_handler->entries.rend());
                        }
                        else
                        {
                            uint32_t frame_id_new = std::stoi(can_grid_tx->m_grid->GetCellValue(wxGridCellCoords(i + 1, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
                            auto this_entry = std::find_if(can_handler->entries.begin(), can_handler->entries.end(), [&frame_id](const auto& item) { return item->id == frame_id; });
                            auto new_entry = std::find_if(can_handler->entries.begin(), can_handler->entries.end(), [&frame_id_new](const auto& item) { return item->id == frame_id_new; });

                            std::iter_swap(this_entry, new_entry);
                            selection = std::distance(can_handler->entries.begin(), new_entry);
                        }
                    }
                }

                RefreshTx();
                m_grid->SelectRow(selection);
            });
        h_sizer->Add(m_MoveDown);

        m_Delete = new wxButton(this, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
        m_Delete->SetToolTip("Delete selected CAN frame(s) from TX list");
        m_Delete->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    uint32_t frame_id = std::stoi(can_grid_tx->m_grid->GetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
                    const CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    {
                        std::scoped_lock lock{ can_handler->m };
                        std::erase_if(can_handler->entries, [frame_id](auto& item) { return item->id == frame_id;  });
                    }
                }

                RefreshTx();
            });
        h_sizer->Add(m_Delete);

        RefreshGuiIconsBasedOnSettings();

        bSizer1->Add(h_sizer);
        bSizer1->AddSpacer(1);

        wxBoxSizer* h_sizer_2 = new wxBoxSizer(wxHORIZONTAL);

        m_SendDataFrame = new wxButton(this, wxID_ANY, "Send Data Frame", wxDefaultPosition, wxDefaultSize);
        m_SendDataFrame->SetToolTip("Send custom Data Frame without adding it to the list");
        m_SendDataFrame->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxTextEntryDialog d(this, "Enter data to send\nExample: [FrameID] [Byte1] [Byte2] [ByteX] ...", "Send Data Frame");
                if(!m_LastDataInput.empty())
                    d.SetValue(m_LastDataInput);
                int ret = d.ShowModal();
                if(ret == wxID_OK)
                {
                    m_LastDataInput = d.GetValue().ToStdString();

                    uint32_t frame_id = 0;
                    char hex[MAX_ISOTP_FRAME_LEN];
                    int ret = sscanf(m_LastDataInput.c_str(), "%x%*c%4095[^\n]", &frame_id, hex);
                    if(ret == 2)
                    {
                        std::string hex_str(hex);
                        char byte_array[MAX_ISOTP_FRAME_LEN];

                        boost::algorithm::erase_all(hex_str, " ");
                        utils::ConvertHexStringToBuffer(hex_str, std::span{ byte_array });

                        uint16_t len = (hex_str.length() / 2);
                        can_handler->SendDataFrame(frame_id, (uint8_t*)byte_array, len);
                        LOG(LogLevel::Notification, "Sending Data Frame, ID: {:X}, Len: {}", frame_id, len);
                    }
                    else
                    {
                        LOG(LogLevel::Notification, "Invalid data format for Data Frame");
                    }
                }
            });

        h_sizer_2->Add(m_SendDataFrame);
        //h_sizer_2->AddSpacer(100);

        m_SendIsoTp = new wxButton(this, wxID_ANY, "Send ISO-TP", wxDefaultPosition, wxDefaultSize);
        m_SendIsoTp->SetToolTip("Send ISO-TP data frame");
        m_SendIsoTp->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                m_UdsRawDialog->ShowDialog();
            });
        h_sizer_2->Add(m_SendIsoTp);

        bSizer1->Add(h_sizer_2);

        wxBoxSizer* h_sizer_3 = new wxBoxSizer(wxHORIZONTAL);
        m_ClearRx = new wxButton(this, wxID_ANY, "Clear RX", wxDefaultPosition, wxDefaultSize);
        m_ClearRx->SetToolTip("Clear RX grid");
        m_ClearRx->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                {
                    std::scoped_lock lock{ can_handler->m };
                    can_handler->m_rxData.clear();
                }
                can_grid_rx->ClearGrid();
            });
        h_sizer_3->Add(m_ClearRx);
        bSizer1->AddSpacer(35);
        bSizer1->Add(h_sizer_3);
    }

    SetSizer(bSizer1);
    Show();
}

void CanSenderPanel::On10MsTimer()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };
    if(can_grid_tx->m_grid->GetNumberRows())
        can_grid_tx->m_grid->DeleteRows(0, can_grid_tx->m_grid->GetNumberRows());
    can_grid_tx->cnt = 0;
    can_grid_tx->grid_to_entry.clear();

    uint8_t default_favourite_level = can_handler->GetFavouriteLevel();
    if(search_pattern_tx.empty())
    {
        for(auto& i : can_handler->entries)
        {
            if(default_favourite_level <= i->favourite_level)
                can_grid_tx->AddRow(i);
        }
    }
    else
    {
        for(auto& i : can_handler->entries)
        {
            if(default_favourite_level <= i->favourite_level)
            {
                if(boost::icontains(i->comment, search_pattern_tx))
                    can_grid_tx->AddRow(i);
            }
        }
    }
}

void CanSenderPanel::RefreshRx()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };

    for(int i = 0; i != can_grid_rx->m_grid->GetNumberRows(); i++)
    {
        uint32_t frame_id = std::stoi(can_grid_rx->m_grid->GetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_Id)).ToStdString(), nullptr, 16);
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
            case CanSenderGridCol::Sender_LogLevel:
            {
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                wxString log_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_LogLevel);
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                try
                {
                    uint8_t log_level = static_cast<uint8_t>(std::stoi(log_str.ToStdString()));
                    can_handler->m_rxData[frame_id]->log_level = log_level;
                    can_handler->m_RxLogLevels[frame_id] = log_level;
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "stoi exception: {}", e.what());
                    can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", can_handler->m_rxData[frame_id]->log_level));
                }
                break;
            }
            case CanSenderGridCol::Sender_FavouriteLevel:
            {
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                wxString fav_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_FavouriteLevel);
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                try
                {
                    uint8_t fav_level = static_cast<uint8_t>(std::stoi(fav_str.ToStdString()));
                    can_handler->m_rxData[frame_id]->favourite_level = fav_level;
                    //can_handler->m_RxLogLevels[frame_id] = log_level;
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "stoi exception: {}", e.what());
                    can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_FavouriteLevel), wxString::Format("%d", can_handler->m_rxData[frame_id]->favourite_level));
                }
                break;
            }
            case CanSenderGridCol::Sender_Comment:
            {
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
            case CanSenderGridCol::Sender_LogLevel:
            {
                try
                {
                    size_t log_level = static_cast<size_t>(std::stoi(new_value.ToStdString()));
                    if(log_level > std::numeric_limits<uint8_t>::max())
                    {
                        log_level = std::numeric_limits<uint8_t>::max();
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", std::numeric_limits<uint8_t>::max()));
                    }
                    can_grid_tx->grid_to_entry[row]->log_level = static_cast<uint8_t>(log_level);
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "stoi exception: {}", e.what());
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", can_grid_tx->grid_to_entry[row]->log_level));
                }
                break;
            }
            case CanSenderGridCol::Sender_FavouriteLevel:
            {
                try
                {
                    size_t fav_level = static_cast<size_t>(std::stoi(new_value.ToStdString()));
                    if(fav_level > std::numeric_limits<uint8_t>::max())
                    {
                        fav_level = std::numeric_limits<uint8_t>::max();
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_FavouriteLevel), wxString::Format("%d", std::numeric_limits<uint8_t>::max()));
                    }
                    can_grid_tx->grid_to_entry[row]->favourite_level = static_cast<uint8_t>(fav_level);
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "stoi exception: {}", e.what());
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_FavouriteLevel), wxString::Format("%d", can_grid_tx->grid_to_entry[row]->favourite_level));
                }
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
/*
void CanSenderPanel::OnCellLeftClick(wxGridEvent& ev)
{
    DBG("left click");
    ev.Skip();
}

void CanSenderPanel::OnCellLeftDoubleClick(wxGridEvent& ev)
{
    DBG("left dclick");
    int row = ev.GetRow(), col = ev.GetCol();

    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
        can_grid_tx->m_grid->SetReadOnly(row, col, can_grid_tx->m_grid->IsReadOnly(row, col));
    ev.Skip();
}
*/
void CanSenderPanel::OnCellRightClick(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_rx->m_grid))
    {
        wxMenu menu;
        menu.Append(ID_CanSenderMoreInfo, "&Show bits")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_CanSenderLogForFrame, "&Log")->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_CanSenderRemoveRxFrame, "&Remove")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
        int ret = GetPopupMenuSelectionFromUser(menu);

        switch(ret)
        {
            case ID_CanSenderMoreInfo:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                CanBitfieldInfo info = can_handler->GetMapForFrameId(frame_id, true);
                if(info.size() == 0)
                {
                    wxMessageDialog(this, "There are no mapping found for selected CAN Frame", "Error", wxOK).ShowModal();
                    return;
                }

                m_BitfieldEditor->ShowDialog(frame_id, true, info);
                break;
            }
            case ID_CanSenderLogForFrame:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                std::vector<std::string> logs;
                can_handler->GenerateLogForFrame(frame_id, true, logs);

                if(logs.empty())
                {
                    wxMessageDialog(this, "In order to see the logs for frames, enable Recording in Log panel", "Error", wxOK).ShowModal();
                }
                else
                    m_LogForFrame->ShowDialog(logs);
                break;
            }
            case ID_CanSenderRemoveRxFrame:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                can_handler->m_rxData.erase(frame_id);  /* Remove this entry from CanEntryHandler's map */
                can_grid_rx->ClearGrid();
                break;
            }
        }
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
    {
        wxMenu menu;
        menu.Append(ID_CanSenderMoreInfo, "&Edit bits")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_CanSenderLogForFrame, "&Log")->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, FromDIP(wxSize(14, 14))));
        int ret = GetPopupMenuSelectionFromUser(menu);

        switch(ret)
        {
            case ID_CanSenderMoreInfo:
            {
    #if 0
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_tx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                CanBitfieldInfo info = can_handler->GetMapForFrameId(frame_id, false);
                m_BitfieldEditor->ShowDialog(frame_id, false, info);

    #endif
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_tx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                bool to_exit = false;
                while(!to_exit)
                {
                    CanBitfieldInfo info = can_handler->GetMapForFrameId(frame_id, false);
                    if(info.size() == 0)
                    {
                        wxMessageDialog(this, "There are no mapping found for selected CAN Frame", "Error", wxOK).ShowModal();
                        return;
                    }

                    m_BitfieldEditor->ShowDialog(frame_id, false, info);
                    if(m_BitfieldEditor->GetClickType() == BitEditorDialog::ClickType::Apply || m_BitfieldEditor->GetClickType() == BitEditorDialog::ClickType::Ok)
                    {
                        std::vector<std::string> ret = m_BitfieldEditor->GetOutput();
                        can_handler->ApplyEditingOnFrameId(frame_id, ret);

                        std::string hex;
                        utils::ConvertHexBufferToString(can_grid_tx->grid_to_entry[row]->data, hex);
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Data), wxString(hex));
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_DataSize),
                            wxString::Format("%lld", can_grid_tx->grid_to_entry[row]->data.size()));
                    }

                    if(m_BitfieldEditor->GetClickType() != BitEditorDialog::ClickType::Apply)
                    {
                        to_exit = true;
                        break;
                    }
                }
                break;
            }
            case ID_CanSenderLogForFrame:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxString frame_str = can_grid_tx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                std::vector<std::string> logs;
                can_handler->GenerateLogForFrame(frame_id, false, logs);

                if(logs.empty())
                {
                    wxMessageDialog(this, "In order to see the logs for frames, enable Recording in Log panel", "Error", wxOK).ShowModal();
                    return;
                }

                m_LogForFrame->ShowDialog(logs);
                break;
            }
        }
    }
}

void CanSenderPanel::OnGridLabelRightClick(wxGridEvent& ev)
{
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_rx->m_grid) || ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
    {
        wxMenu menu;
        menu.Append(ID_CanSenderEditLogLevel, "&Edit log level")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_CanSenderEditFavourites, "&Edit favourites")->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, FromDIP(wxSize(14, 14))));
        int ret = GetPopupMenuSelectionFromUser(menu);
        switch(ret)
        {
            case ID_CanSenderEditLogLevel:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxTextEntryDialog d(this, "Enter default log level for TX & RX list", "Default log level");
                d.SetValue(std::to_string(can_handler->GetRecordingLogLevel()));
                int ret = d.ShowModal();
                if(ret == wxID_OK)
                {
                    uint8_t loglevel = 0;
                    try
                    {
                        loglevel = std::stoi(d.GetValue().ToStdString());
                    }
                    catch(const std::exception& e)
                    {
                        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
                    }
                    can_handler->SetRecordingLogLevel(loglevel);
                }
                break;
            }
            case ID_CanSenderEditFavourites:
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxTextEntryDialog d(this, "Enter default favourite level for TX & RX list", "Default favourite level");
                d.SetValue(std::to_string(can_handler->GetFavouriteLevel()));
                int ret = d.ShowModal();
                if(ret == wxID_OK)
                {
                    uint8_t favourite_level = 0;
                    try
                    {
                        favourite_level = std::stoi(d.GetValue().ToStdString());
                    }
                    catch(const std::exception& e)
                    {
                        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
                    }
                    can_handler->SetFavouriteLevel(favourite_level);

                    RefreshTx();
                }

                break;
            }
        }
    }
}

void CanSenderPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CanSenderPanel::LoadTxList()
{
    wxFileDialog openFileDialog(this, _("Open TX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_tx = openFileDialog.GetPath();
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_rx.ToStdString();
    can_handler->SaveRxList(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListSaved) });
}

void CanSenderPanel::LoadMapping()
{
    wxFileDialog openFileDialog(this, _("Open FrameMapping XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_mapping = openFileDialog.GetPath();
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_mapping.ToStdString();
    bool ret = can_handler->LoadMapping(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(ret ? PopupMsgIds::FrameMappingLoaded : PopupMsgIds::FrameMappingLoadError) });
}

void CanSenderPanel::SaveMapping()
{
    wxFileDialog saveFileDialog(this, _("Save FrameMapping XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_mapping = saveFileDialog.GetPath();
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_mapping.ToStdString();
    bool ret = can_handler->SaveMapping(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::FrameMappingSaved) });
}

void CanSenderPanel::OnKeyDown(wxKeyEvent& evt)
{
    if(evt.GetKeyCode() == WXK_F2)
    {
        DBG("f2");

        wxArrayInt rows = can_grid_tx->m_grid->GetSelectedRows();
        wxArrayInt cols = can_grid_tx->m_grid->GetSelectedCols();
        if(rows.empty() || rows.size() > 1 || cols.empty() || cols.size() > 1) return;

        int row = rows[0];
        int col = cols[0];

        can_grid_tx->m_grid->SetReadOnly(row, col, true);
    }

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
                    return;
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
                    return;
                }
                break;
            }
            case 'B':  /* Show bits */
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == can_grid_rx->m_grid)
                {
                    wxArrayInt rows = can_grid_rx->m_grid->GetSelectedRows();
                    if(rows.empty() || rows.size() > 1) return;

                    wxString frame_str = can_grid_rx->m_grid->GetCellValue(rows[0], CanSenderGridCol::Sender_Id);
                    uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                    CanBitfieldInfo info = can_handler->GetMapForFrameId(frame_id, true);
                    if(info.size() == 0)
                    {
                        wxMessageDialog(this, "There are no mapping found for selected CAN Frame", "Error", wxOK).ShowModal();
                        evt.Skip();
                        return;
                    }

                    m_BitfieldEditor->ShowDialog(frame_id, true, info);
                }
                else
                {
                    wxArrayInt rows = can_grid_tx->m_grid->GetSelectedRows();
                    if(rows.empty() || rows.size() > 1) return;

                    int row = rows[0];
                    wxString frame_str = can_grid_tx->m_grid->GetCellValue(row, CanSenderGridCol::Sender_Id);
                    uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                    CanBitfieldInfo info = can_handler->GetMapForFrameId(frame_id, false);
                    if(info.size() == 0)
                    {
                        wxMessageDialog(this, "There are no mapping found for selected CAN Frame", "Error", wxOK).ShowModal();
                        evt.Skip();
                        return;
                    }
                    /*
                    m_BitfieldEditor->ShowDialog(frame_id, false, info);
                    if(m_BitfieldEditor->IsApplyClicked())
                    {
                        std::vector<std::string> ret = m_BitfieldEditor->GetOutput();

                        can_handler->ApplyEditingOnFrameId(frame_id, ret);

                        std::string hex;
                        utils::ConvertHexBufferToString(can_grid_tx->grid_to_entry[row]->data, hex);
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Data), wxString(hex));
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_DataSize),
                            wxString::Format("%lld", can_grid_tx->grid_to_entry[row]->data.size()));
                    }*/

                }
                break;
            }
            case 'L':  /* Show log */
            {
                std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == can_grid_rx->m_grid)
                {
                    wxArrayInt rows = can_grid_rx->m_grid->GetSelectedRows();
                    if(rows.empty() || rows.size() > 1) return;

                    wxString frame_str = can_grid_rx->m_grid->GetCellValue(rows[0], CanSenderGridCol::Sender_Id);
                    uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                    std::vector<std::string> logs;
                    can_handler->GenerateLogForFrame(frame_id, true, logs);

                    if(logs.empty())
                    {
                        wxMessageDialog(this, "In order to see the logs for frames, enable Recording in Log panel", "Error", wxOK).ShowModal();
                    }
                    else
                        m_LogForFrame->ShowDialog(logs);
                }
                else
                {
                    wxArrayInt rows = can_grid_tx->m_grid->GetSelectedRows();
                    if(rows.empty() || rows.size() > 1) return;

                    wxString frame_str = can_grid_tx->m_grid->GetCellValue(rows[0], CanSenderGridCol::Sender_Id);
                    uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                    std::vector<std::string> logs;
                    can_handler->GenerateLogForFrame(frame_id, false, logs);

                    if(logs.empty())
                    {
                        wxMessageDialog(this, "In order to see the logs for frames, enable Recording in Log panel", "Error", wxOK).ShowModal();
                        return;
                    }

                    m_LogForFrame->ShowDialog(logs);
                }
                break;
            }
        }
    }
    evt.Skip();
}

void CanSenderPanel::UpdateGridForTxFrame(uint32_t frame_id, uint8_t* buffer)
{
    std::string hex_str;
    utils::ConvertHexBufferToString((const char*)buffer, 8, hex_str);

    for(int i = 0; i != can_grid_tx->cnt; i++)
    {
        if(can_grid_tx->grid_to_entry[i]->id == frame_id)
        {
            can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(i, Sender_Data), wxString(hex_str));
            can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(i, CanSenderGridCol::Sender_DataSize), wxString::Format("%lld", hex_str.length() / 2));
            //can_grid_tx->grid_to_entry[row]->data.assign(bytes, bytes + (hex_str.length() / 2));
            break;
        }
    }
}