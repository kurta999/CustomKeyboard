#include "pch.hpp"

#include <wx/headerctrl.h>

wxBEGIN_EVENT_TABLE(CanPanel, wxPanel)
EVT_SIZE(CanPanel::OnSize)
wxEND_EVENT_TABLE()

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

wxBEGIN_EVENT_TABLE(CanLogPanel, wxPanel)
EVT_SIZE(CanLogPanel::OnSize)
EVT_CHAR_HOOK(CanLogPanel::OnKeyDown)
EVT_SPINCTRL(ID_CanLogLevelSpinCtrl, CanLogPanel::OnLogLevelChange)
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

CanLogPanel::CanLogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Log :: TX: 0, RX: 0, Total: 0");
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(*wxBLUE);

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600), 0);

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
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(true, false);
        });
    h_sizer->Add(m_RecordingStart);

    m_RecordingPause = new wxButton(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize);
    m_RecordingPause->SetToolTip("Suspend recording for received & sent CAN frames");
    m_RecordingPause->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, true);
        });
    h_sizer->Add(m_RecordingPause);

    m_RecordingStop = new wxButton(this, wxID_ANY, "Stop", wxDefaultPosition, wxDefaultSize);
    m_RecordingStop->SetToolTip("Suspend recording for received & sent CAN frames, clear everything");
    m_RecordingStop->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, false);
            inserted_until = 0;
        });
    h_sizer->Add(m_RecordingStop);

    m_RecordingClear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_RecordingClear->SetToolTip("Clear recording and reset frame counters");
    m_RecordingClear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            ClearRecordingsFromGrid();

            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ClearRecording();
        });
    h_sizer->Add(m_RecordingClear);
    //h_sizer->AddSpacer(35);

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

    h_sizer->AddSpacer(10);
    h_sizer->Add(new wxStaticText(this, wxID_ANY, "LogLevel:"));
    m_LogLevelCtrl = new wxSpinCtrl(this, ID_CanLogLevelSpinCtrl, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 1);
    h_sizer->Add(m_LogLevelCtrl);

    m_RecordingSave = new wxButton(this, wxID_ANY, "Save log", wxDefaultPosition, wxDefaultSize);
    m_RecordingSave->SetToolTip("Save recording");
    m_RecordingSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
#ifdef _WIN32
            const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

            if(std::filesystem::exists("Can"))
                std::filesystem::create_directory("Can");
            std::string log_format = std::format("Can/CanLog_{:%Y.%m.%d_%H_%M_%OS}.csv", now);
            std::filesystem::path p(log_format);

            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
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
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    //std::scoped_lock lock{ can_handler->m };

    static std::string last_search_pattern;
    static uint64_t last_tx_cnt = 0, last_rx_cnt = 0;

    if(search_pattern.empty())
    {
        if(last_tx_cnt != can_handler->GetTxFrameCount() || last_rx_cnt != can_handler->GetRxFrameCount())
        {
            static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: TX: %lld, RX: %lld, Total: %lld", can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(),
                can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
        }
    }
    else
    {
        if(last_tx_cnt != can_handler->GetTxFrameCount() || last_rx_cnt != can_handler->GetRxFrameCount() || last_search_pattern != search_pattern)
        {
            static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: Filter: %s, TX: %lld, RX: %lld, Total: %lld", search_pattern,
                can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(), can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
        }
    }

    last_tx_cnt = can_handler->GetTxFrameCount();
    last_rx_cnt = can_handler->GetRxFrameCount();
    last_search_pattern = search_pattern;

    if(!can_handler->m_LogEntries.empty())
    {
        if(!is_something_inserted)
            inserted_until = 0;

        auto it = can_handler->m_LogEntries.begin() + inserted_until;
        //std::advance(it, inserted_until);
        if(it == can_handler->m_LogEntries.end())
        {
            DBG("shit happend");
            return;
        }

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

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - can_handler->GetStartTime()).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Data), hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Direction), direction == CAN_LOG_DIR_TX ? "TX" : "RX");
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Id), wxString::Format("%X", id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_DataSize), wxString::Format("%lld", data.size()));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Comment), comment);

    if(m_AutoScroll)
        m_grid->ScrollLines(num_rows);

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
    script = new CanScriptPanel(this);
    m_notebook->Freeze();
    m_notebook->AddPage(sender, "Sender", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(log, "Log", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(script, "Script", false, wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(CanPanel::Changeing), NULL, this);
    
    /* size: 1640x1080 */
    m_notebook->Split(0, wxLEFT);
    
    m_notebook->Thaw();
    m_notebook->SetAutoLayout(true);
    m_notebook->Layout();
    m_notebook->SetSize(m_notebook->GetSize());
    m_notebook->SetSelection(0);
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

void CanPanel::LoadMapping()
{
    sender->LoadMapping();
}

void CanPanel::SaveMapping()
{
    sender->SaveMapping();
}

void CanPanel::On10MsTimer()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    bool is_lock_ok = can_handler->m.try_lock();
    if(!is_lock_ok)
    {
        DBG("\n\nCanSenderPanel::On10MsTimer lock failed\n\n");
        return;
    }
    sender->On10MsTimer();
    log->On10MsTimer();
    can_handler->m.unlock();
}

void CanSenderPanel::On10MsTimer()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    //std::scoped_lock lock{ can_handler->m };
    /*
    bool is_lock_ok = can_handler->m.try_lock();
    if(!is_lock_ok)
    {
        DBG("\n\nCanSenderPanel::On10MsTimer lock failed\n\n");
        return;
    }
    */
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
    //can_handler->m.unlock();
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
                }

                break;
            }
        }
    }
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

void CanPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CanSenderPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CanLogPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
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
                break;
            }
            case 'C':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == m_grid)
                {
                    wxArrayInt rows = m_grid->GetSelectedRows();
                    if(rows.empty()) return;

                    wxString str_to_copy;
                    for(auto& row : rows)
                    {
                        for(uint8_t col = 0; col < CanSenderGridCol::Sender_Max - 1; col++)
                        {
                            str_to_copy += m_grid->GetCellValue(row, col);
                            str_to_copy += '\t';
                        }
                        str_to_copy += '\n';
                    }
                    if(str_to_copy.Last() == '\n')
                        str_to_copy.RemoveLast();

                    if(wxTheClipboard->Open())
                    {
                        wxTheClipboard->SetData(new wxTextDataObject(str_to_copy));
                        wxTheClipboard->Close();
                        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
                        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::SelectedLogsCopied) });
                    }
                }
                break;
            }
        }
    }
}

void CanLogPanel::OnLogLevelChange(wxSpinEvent& evt)
{
    uint8_t new_log_level = static_cast<uint8_t>(evt.GetValue());
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    can_handler->SetRecordingLogLevel(new_log_level);
}

wxBEGIN_EVENT_TABLE(CanScriptPanel, wxPanel)
EVT_FILEPICKER_CHANGED(ID_FilePickerCanScript, CanScriptPanel::OnFileSelected)
EVT_SIZE(CanScriptPanel::OnSize)
wxEND_EVENT_TABLE()

CanScriptPanel::CanScriptPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    m_Script = std::make_unique<CanScriptHandler>(*this);
    can_handler->m_ScriptHandler = m_Script.get();

    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Select scipt file, paste it's content or Drag'n'Drop to textbox below\nWhen done, click on Run!"), wxDefaultPosition, wxDefaultSize, 0));
	m_FilePicker = new wxFilePickerCtrl(this, ID_FilePickerCanScript, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxSize(500, 25), wxFLP_DEFAULT_STYLE);
	bSizer1->Add(m_FilePicker);

	m_Input = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(410, 410), wxTE_MULTILINE);
#ifdef _WIN32
	m_Input->DragAcceptFiles(true);  /* This one doesn't work with GTK for some reason... */
#endif
	m_Input->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(CanScriptPanel::OnFileDrop), NULL, this);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	bSizer2->Add(m_Input, wxSizerFlags(1).Top().Expand());

	m_Output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(310, 310), wxTE_MULTILINE);
	bSizer2->Add(m_Output, wxSizerFlags(1).Expand());
	bSizer1->Add(bSizer2, wxSizerFlags(1).Expand());

	wxBoxSizer* h_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
    m_RunButton = new wxButton(this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_RunButton);
    m_RunSelectedButton = new wxButton(this, wxID_ANY, wxT("Run selected"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_RunSelectedButton);

    m_Abort = new wxButton(this, wxID_ANY, wxT("Abort"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_Abort);
    m_Abort->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            wxString str_sel = m_Input->GetStringSelection();

            m_Script->AbortRunningScript();
        });

	h_sizer_2->AddSpacer(100);
	
	m_ClearButton = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_ClearButton);
	m_ClearButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			m_Input->Clear();
			m_Output->Clear();
			m_FilePicker->SetFileName(wxFileName());
            m_RunButton->SetForegroundColour(*wxBLACK);
			path.Clear();
		});

    m_ClearOutput = new wxButton(this, wxID_ANY, wxT("Clear output"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_ClearOutput);
    m_ClearOutput->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			m_Output->Clear();
			path.Clear();
		});

	bSizer1->Add(h_sizer_2, wxSizerFlags(1).Expand());

	this->SetSizerAndFit(bSizer1);
	this->Layout();

    m_RunButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            if(m_Script->IsScriptRunning())
            {
                wxMessageDialog(this, std::format("Click on abort to abort it, before running another one"), "A script is already running", wxOK).ShowModal();
                return;
            }

			wxString str = m_Input->GetValue();
			std::string input;
			if(!path.empty())
			{
				LOG(LogLevel::Verbose, "Path: {}", path.ToStdString());
				std::ifstream f(path.ToStdString(), std::ios::in | std::ios::binary);
				if(f)
					input = { (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>() };
			}
			else
			{
				input = str.mb_str();
			}

			try
			{
                boost::algorithm::replace_all(input, "\r", "");  /* Thanks Windows */
                m_Script->RunScript(input);
			}
			catch(std::exception& e)
			{
				LOG(LogLevel::Error, "Exception: {}", e.what());
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}

			if(!input.empty())
			{
#ifdef _WIN32
				boost::algorithm::replace_all(input, "\n", "\r\n");  /* LF isn't enough for TextCtrl for some reason... */
#endif
				m_Output->Clear();
                m_Output->SetValue("");
                m_RunButton->SetForegroundColour(*wxBLACK);
			}
			else
			{
				LOG(LogLevel::Verbose, "Empty input");
			}
			path.Clear();
		});

    m_RunSelectedButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            if(m_Script->IsScriptRunning())
            {
                wxMessageDialog(this, std::format("Click on abort to abort it, before running another one"), "A script is already running", wxOK).ShowModal();
                return;
            }

            std::string input = m_Input->GetStringSelection().ToStdString();;
            if(input.empty())
            {
                LOG(LogLevel::Warning, "Empty selection, nothing to run");
                return;
            }

            try
            {
                boost::algorithm::replace_all(input, "\r", "");  /* Thanks Windows */
                m_Script->RunScript(input);
            }
            catch(std::exception& e)
            {
                LOG(LogLevel::Error, "Exception: {}", e.what());
                wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
            }

            if(!input.empty())
            {
#ifdef _WIN32
                boost::algorithm::replace_all(input, "\n", "\r\n");  /* LF isn't enough for TextCtrl for some reason... */
#endif
                m_Output->Clear();
                m_Output->SetValue("");
                m_RunButton->SetForegroundColour(*wxBLACK);
            }
            else
            {
                LOG(LogLevel::Verbose, "Empty input");
            }
            path.Clear();
        });
}

void CanScriptPanel::AddToLog(std::string str)
{
    m_Output->AppendText(str);
}

void CanScriptPanel::OnFileDrop(wxDropFilesEvent& event)
{
    if(event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        path = *dropped;
        m_FilePicker->SetFileName(wxFileName(path));

        HandleInputFileSelect(path);
    }
}

void CanScriptPanel::OnFileSelected(wxFileDirPickerEvent& event)
{
    path = event.GetPath();
    HandleInputFileSelect(path);
}

void CanScriptPanel::OnSize(wxSizeEvent& event)
{
    wxSize a = event.GetSize();
    event.Skip(true);
}

void CanScriptPanel::HandleInputFileSelect(wxString& path)
{
    std::ifstream f(path.ToStdString(), std::ios::in | std::ios::binary);
    if(f)
    {
        std::string input = { (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>() };
        if(!input.empty())
        {
#ifdef _WIN32
            boost::algorithm::replace_all(input, "\n", "\r\n");  /* LF isn't enough for TextCtrl for some reason... */
#endif
            m_RunButton->SetForegroundColour(*wxRED);
            m_Input->SetValue(input);
        }
    }
}

wxBEGIN_EVENT_TABLE(BitEditorDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, BitEditorDialog::OnApply)
EVT_BUTTON(wxID_OK, BitEditorDialog::OnOk)
EVT_BUTTON(wxID_CLOSE, BitEditorDialog::OnCancel)
EVT_CLOSE(BitEditorDialog::OnClose)
wxEND_EVENT_TABLE()

BitEditorDialog::BitEditorDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Bit editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);
    sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Bit editor");
    
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_IsDecimal = new wxRadioButton(this, wxID_ANY, "Decimal");
    m_IsDecimal->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsDecimal);
    m_IsHex = new wxRadioButton(this, wxID_ANY, "Hex");
    m_IsHex->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsHex);
    m_IsBinary = new wxRadioButton(this, wxID_ANY, "Binary");
    m_IsBinary->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsBinary);

    sizerMsgs->Add(h_sizer);
    sizerMsgs->AddSpacer(20);

    for(int i = 0; i != MAX_BITEDITOR_FIELDS; i++)
    {
        m_InputLabel[i] = new wxStaticText(this, wxID_ANY, "_");
        sizerMsgs->Add(m_InputLabel[i], 1, wxLEFT | wxEXPAND, 0);
        m_Input[i] = new wxTextCtrl(this, wxID_ANY, "_", wxDefaultPosition, wxSize(250, 25), 0);
        sizerMsgs->Add(m_Input[i], 1, wxLEFT | wxEXPAND, 0);
    }

    sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE | wxOK), wxSizerFlags().Right().Border()); /* wxOK */
     
    sizerTop->SetMinSize(wxSize(200, 200));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    //sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

void BitEditorDialog::ShowDialog(uint32_t frame_id, bool is_rx, CanBitfieldInfo& values)
{
    if(values.size() > MAX_BITEDITOR_FIELDS)
    {
        values.resize(MAX_BITEDITOR_FIELDS);
        LOG(LogLevel::Warning, "Too much bitfields used for can frame mapping. FrameID: {:X}, Used: {}, Maximum supported: {}", frame_id, values.size(), MAX_BITEDITOR_FIELDS);
    }

    m_Id = 0;
    m_DataFormat = 0;
    m_BitfieldInfo = values;
    
    for(const auto& [label, value, frame] : values)
    {
        m_InputLabel[m_Id]->SetLabelText(label);

        m_InputLabel[m_Id]->SetForegroundColour(RGB_TO_WXCOLOR(frame->m_color));  /* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
        m_InputLabel[m_Id]->SetBackgroundColour(RGB_TO_WXCOLOR(frame->m_bg_color));

        wxFont font;
        font.SetWeight(frame->m_is_bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
        font.Scale(1.0f);  /* Scale has to be set to default first */
        m_InputLabel[m_Id]->SetFont(font);
        font.Scale(frame->m_scale);
        font.SetFaceName("Segoe UI");
        m_InputLabel[m_Id]->SetFont(font);

        m_InputLabel[m_Id]->Show();
        m_InputLabel[m_Id]->SetToolTip(frame->m_Description);
        m_Input[m_Id]->SetValue(value);
        m_Input[m_Id]->Show();
        m_Id++;
    }

    for(int i = m_Id; i != MAX_BITEDITOR_FIELDS; i++)
    {
        m_InputLabel[i]->Hide();
        m_InputLabel[i]->SetToolTip("");
        m_Input[i]->Hide();
    }

    SetTitle(wxString::Format("Bit editor - %X (%s)", frame_id, is_rx ? "RX" : "TX"));
    bit_sel = BitSelection::Decimal;
    m_IsDecimal->SetValue(true);
    m_IsHex->SetValue(false);
    m_IsBinary->SetValue(false);
    m_FrameId = frame_id;

    sizerTop->Layout();
    sizerTop->Fit(this);

    m_ClickType = ClickType::None;
    int ret = ShowModal();
    DBG("ShowModal ret: %d\n", ret);
}

std::vector<std::string> BitEditorDialog::GetOutput()
{
    std::vector<std::string> ret;
    for(int i = 0; i != m_Id; i++)
    {
        std::string input_ret = m_Input[i]->GetValue().ToStdString();
        switch(bit_sel)
        {
            case BitSelection::Hex:
            {
                try
                {
                    input_ret = std::to_string(std::stoll(input_ret, nullptr, 16));
                }
                catch(...)
                {
                    LOG(LogLevel::Error, "Exception with std::stoll, str: {}", input_ret);
                }
                break;
            }
            case BitSelection::Binary:
            {
                try
                {
                    input_ret = std::to_string(std::stoll(input_ret, nullptr, 2));
                }
                catch(...)
                {
                    LOG(LogLevel::Error, "Exception with std::to_string, str: {}", input_ret);
                }
                break;
            }
        }

        DBG("input_ret: %s\n", input_ret.c_str());
        ret.push_back(input_ret);
    }
    return ret;
}

void BitEditorDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnApply %d\n", (int)m_ClickType)
    EndModal(wxID_APPLY);
    //Close();
    m_ClickType = ClickType::Apply;
}

void BitEditorDialog::OnOk(wxCommandEvent& event)
{
    DBG("OnOK %d\n", (int)m_ClickType);
    if(m_ClickType == ClickType::Close)
        return;
    EndModal(wxID_OK);
    //Close();
    m_ClickType = ClickType::Ok;
    event.Skip();
}

void BitEditorDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnCancel %d\n", (int)m_ClickType);
    EndModal(wxID_CLOSE);
    m_ClickType = ClickType::Close;
    //Close();
}

void BitEditorDialog::OnClose(wxCloseEvent& event)
{
    DBG("OnClose %d\n", (int)m_ClickType);
    m_ClickType = ClickType::Close;
    EndModal(wxID_CLOSE);
}

void BitEditorDialog::OnRadioButtonClicked(wxCommandEvent& event)
{
    if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsDecimal))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            m_Input[cnt]->SetValue(value);
            if(++cnt > m_Id)
                break;
        }
        bit_sel = BitSelection::Decimal;
    }
    else if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsHex))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if(utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:X}", decimal_val);  /* std::to_chars gives lower case letters, I don't like it :/ */

                m_Input[cnt]->SetValue(hex_str);
                if(++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Hex;
    }
    else if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsBinary))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if(utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:b}", decimal_val);

                m_Input[cnt]->SetValue(hex_str);
                if(++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Binary;
    }
}

wxBEGIN_EVENT_TABLE(CanLogForFrameDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, CanLogForFrameDialog::OnApply)
wxEND_EVENT_TABLE()

CanLogForFrameDialog::CanLogForFrameDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "CAN log for frame", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);

    m_Log = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB);
    m_Log->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
        {
            wxClipboard* clip = new wxClipboard();
            clip->Clear();
            clip->SetData(new wxTextDataObject(m_Log->GetString(m_Log->GetSelection())));
            clip->Flush();
            clip->Close();
        });
    sizerTop->Add(m_Log, wxSizerFlags(1).Left().Expand());

    //sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

    sizerTop->SetMinSize(wxSize(640, 480));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

void CanLogForFrameDialog::ShowDialog(std::vector<std::string>& values)
{
    m_Log->Clear();
    for(const auto i : values)
    {
        m_Log->Append(i);
    }
    ShowModal();
}

void CanLogForFrameDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    Close();
    m_Log->Clear();
}

wxBEGIN_EVENT_TABLE(CanUdsRawDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, CanUdsRawDialog::OnApply)
wxEND_EVENT_TABLE()

CanUdsRawDialog::CanUdsRawDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "CAN UDS Raw", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizerMsgs_sender = new wxStaticBoxSizer(wxVERTICAL, this, "&Send");
    wxBoxSizer* h_sizer_0 = new wxBoxSizer(wxHORIZONTAL);
    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Sender FrameID:"));
    m_SenderId = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_SenderId);
    sizerMsgs_sender->AddSpacer(5);

    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Receiver FrameID:"));
    m_ReceiverId = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_ReceiverId);
    //sizerMsgs_sender->Add(h_sizer_0);


    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Frame delay:"));
    m_DelayBetweenFrames = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_DelayBetweenFrames);

    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Recv delay:"));
    m_RecvDelayFrames = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_RecvDelayFrames);
    sizerMsgs_sender->Add(h_sizer_0);

    sizerMsgs_sender->AddSpacer(10);

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_ReadDid = new wxButton(this, wxID_ANY, "Read DID", wxDefaultPosition, wxDefaultSize);
    m_ReadDid->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 03\r\n22 4000");
        });
    h_sizer->Add(m_ReadDid);

    m_WriteDid = new wxButton(this, wxID_ANY, "Write DID", wxDefaultPosition, wxDefaultSize);
    h_sizer->Add(m_WriteDid);
    m_WriteDid->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 03\r\n2E 4000 0C\r\nDELAY 10\r\n22 4000");
        });

    m_EcuReset = new wxButton(this, wxID_ANY, "ECU Reset", wxDefaultPosition, wxDefaultSize);
    h_sizer->Add(m_EcuReset);
    m_EcuReset->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 02");
        });

    m_Clear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_Clear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("");
            m_DataRecv->SetValue("");
        });

    h_sizer->Add(m_Clear);
    sizerMsgs_sender->Add(h_sizer);

    sizerMsgs_sender->AddSpacer(10);

    wxStaticText* data_to_send_text = new wxStaticText(this, wxID_ANY, "UDS Data to send:");
    data_to_send_text->SetToolTip("10 - Diagnostic Session Control\n11 - ECU Reset\n3E - Tester present\n22 - Read Data By Identifier\n23 - Read Memory By Address\n\
24 - Read Scaling Data By Identifier\n2A - Read Data By Identifier Periodic\n2E - Write Data By Identifier\n3D - Write Memory By Address\n14 - Delete all DTC\n\
19 - Read DTC Information");

    sizerMsgs_sender->Add(data_to_send_text);
    m_DataToSend = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 100), wxTE_MULTILINE);
    sizerMsgs_sender->Add(m_DataToSend, 1, wxLEFT | wxEXPAND, 0);

    wxBoxSizer* h_sizer_3 = new wxBoxSizer(wxHORIZONTAL);
    m_SendBtn = new wxButton(this, wxID_ANY, "Send", wxDefaultPosition, wxDefaultSize);
    m_SendBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            HandleFrameSending();
        });

    h_sizer_3->AddStretchSpacer();
    h_sizer_3->Add(m_SendBtn);
    sizerMsgs_sender->Add(h_sizer_3);

    sizerTop->Add(sizerMsgs_sender, wxSizerFlags(1).Expand().Border());

    wxStaticBoxSizer* sizerMsgs_recv = new wxStaticBoxSizer(wxVERTICAL, this, "&Receive");
    wxStaticText* data_to_recv_text = new wxStaticText(this, wxID_ANY, "UDS Response:");
    data_to_recv_text->SetToolTip("11 - serviceNotSupported\n12 - subFunctionNotSupported\n14 - responseTooLong\n21 - busyRepeatReques\n22 - conditionsNotCorrect\n\
31 - requestOutOfRange\n33 - securityAccessDenied\n78 - ResponsePending\n7E - subFunctionNotSupportedInActiveSession\n7F -serviceNotSupportedInActiveSession");

    sizerMsgs_recv->Add(data_to_recv_text);
    m_DataRecv = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 100), wxTE_MULTILINE);
    sizerMsgs_recv->Add(m_DataRecv, 1, wxLEFT | wxEXPAND, 0);
    sizerTop->Add(sizerMsgs_recv, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

    sizerTop->SetMinSize(wxSize(640, 480));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

void CanUdsRawDialog::ShowDialog()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    if(m_LastUdsSenderId == 0)
    {
        m_LastUdsSenderId = can_handler->GetDefaultEcuId();
        m_LastUdsReceiverId = can_handler->GetIsoTpResponseFrameId();
        m_LastDelayBetweenFrames = 100;
        m_LastRecvWaitingTime = 350;
        m_LastUdsInput.clear();
    }

    m_IsApplyClicked = false;
    m_SenderId->SetValue(wxString::Format("%X", m_LastUdsSenderId));
    m_ReceiverId->SetValue(wxString::Format("%X", m_LastUdsReceiverId));
    m_DelayBetweenFrames->SetValue(wxString::Format("%d", m_LastDelayBetweenFrames));
    m_RecvDelayFrames->SetValue(wxString::Format("%d", m_LastRecvWaitingTime));
    m_DataRecv->SetValue("");
    ShowModal();
}

uint32_t CanUdsRawDialog::GetSenderId()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_SenderId->GetValue().ToStdString(), 0, 16);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetReceiverId()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_ReceiverId->GetValue().ToStdString(), 0, 16);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetDelayBetweenFrames()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_DelayBetweenFrames->GetValue().ToStdString());
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetWaitingTimeForFrames()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_RecvDelayFrames->GetValue().ToStdString());
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

std::string CanUdsRawDialog::GetSentData()
{
    return m_DataToSend->GetValue().ToStdString();
}

void CanUdsRawDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    m_IsApplyClicked = true;
    Close();
}

void CanUdsRawDialog::HandleFrameSending()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

    m_LastUdsSenderId = GetSenderId();
    m_LastUdsReceiverId = GetReceiverId();
    m_LastUdsInput = GetSentData();
    m_LastDelayBetweenFrames = GetDelayBetweenFrames();

    auto& uds_responses = can_handler->GetUdsRawBuffer();
    uds_responses.clear();  /* Clear every older request */

    uint32_t old_recv_frame_id = can_handler->GetIsoTpResponseFrameId();
    std::vector<std::string> lines;
    boost::split(lines, m_LastUdsInput, [](char input) { return input == '\n' || input == ';'; }, boost::algorithm::token_compress_on);
    for(auto& hex_str : lines)
    {
        int delay = 0;
        int delay_check = sscanf(hex_str.c_str(), "DELAY%*c%d%*c[^\n]", &delay);
        if(delay_check == 1)
        {
            {
                LOG(LogLevel::Notification, "Sending ISO-TP Frame, Delay: {}ms", delay);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            continue;
        }

        char byte_array[MAX_ISOTP_FRAME_LEN];
        boost::algorithm::erase_all(hex_str, " ");
        boost::algorithm::erase_all(hex_str, ".");
        utils::ConvertHexStringToBuffer(hex_str, std::span{ byte_array });

        uint16_t len = (hex_str.length() / 2);
        if(len == 0)
        {
            LOG(LogLevel::Warning, "Skipping IsoTP frame, input length is zero");
            continue;
        }
        can_handler->SendIsoTpFrame(m_LastUdsSenderId, (uint8_t*)byte_array, len);
        LOG(LogLevel::Notification, "Sending ISO-TP Frame, FrameID: {:X}, ResponseFrameID: {:X}, Len: {}", m_LastUdsSenderId, can_handler->GetIsoTpResponseFrameId(), len);

        std::this_thread::sleep_for(std::chrono::milliseconds(m_LastDelayBetweenFrames));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  /* TODO: use mutex or something else */

    while(can_handler->GetElapsedTimeSinceLastUdsFrame() < m_LastDelayBetweenFrames)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::string response;
    for(auto& i : uds_responses)
    {
        std::string tmp;
        utils::ConvertHexBufferToString(i.c_str(), i.length(), tmp);
        response += tmp + "\r\n";
    }
    uds_responses.clear();
    m_DataRecv->SetValue(response);

    if(old_recv_frame_id != m_LastUdsReceiverId)
        can_handler->SetIsoTpResponseFrame(old_recv_frame_id);
}