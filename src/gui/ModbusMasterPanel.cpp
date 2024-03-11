#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ModbusDataPanel, wxPanel)
EVT_GRID_CELL_CHANGED(ModbusDataPanel::OnCellValueChanged)
EVT_SIZE(ModbusDataPanel::OnSize)
EVT_GRID_CELL_RIGHT_CLICK(ModbusDataPanel::OnCellRightClick)
EVT_GRID_LABEL_LEFT_CLICK(ModbusDataPanel::OnGridLabelLeftClick)
EVT_GRID_LABEL_RIGHT_CLICK(ModbusDataPanel::OnGridLabelRightClick)
EVT_CHAR_HOOK(ModbusDataPanel::OnKeyDown)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusLogPanel, wxPanel)
EVT_SIZE(ModbusLogPanel::OnSize)
EVT_CHAR_HOOK(ModbusLogPanel::OnKeyDown)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusSpecialRegisterPanel, wxPanel)
EVT_SIZE(ModbusSpecialRegisterPanel::OnSize)
EVT_CHAR_HOOK(ModbusSpecialRegisterPanel::OnKeyDown)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusMasterPanel, wxPanel)
EVT_SIZE(ModbusMasterPanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusDataEditDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, ModbusDataEditDialog::OnApply)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ModbusBitEditorDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, ModbusBitEditorDialog::OnApply)
EVT_BUTTON(wxID_OK, ModbusBitEditorDialog::OnOk)
EVT_BUTTON(wxID_CLOSE, ModbusBitEditorDialog::OnCancel)
EVT_CLOSE(ModbusBitEditorDialog::OnClose)
wxEND_EVENT_TABLE()

ModbusItemPanel::ModbusItemPanel(wxWindow* parent, const wxString& header_name, ModbusItemType& items, bool is_read_only)
    : m_items(items), m_isReadOnly(is_read_only)
{
	static_box = new wxStaticBoxSizer(wxVERTICAL, parent, header_name);
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(wxColor(235, 52, 204));

    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(250, 700), 0);

    // Grid
    m_grid->CreateGrid(0, ModbusGridCol::Modbus_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Name, "Name");
    m_grid->SetColLabelValue(ModbusGridCol::Modbus_Value, "Value");

    m_grid->SetColSize(ModbusGridCol::Modbus_Name, 130);

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
    m_grid->SetRowLabelSize(60);

    //m_grid->HideRowLabels();
    static_box->Add(m_grid);

    UpdatePanel();
}

void ModbusItemPanel::AddItem(std::unique_ptr<ModbusItem>& e)
{
    m_grid->AppendRows(1);
    int num_row = m_grid->GetNumberRows() - 1;
    if (e->GetSize() == 1)
        m_grid->SetRowLabelValue(num_row, wxString::Format("%lld", e->m_Offset));
    else
        m_grid->SetRowLabelValue(num_row, wxString::Format("%lld - %lld", e->m_Offset, e->m_Offset + (e->GetSize() - 1)));

    grid_to_entry[num_row] = e.get();

    m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), e->m_Name);
    m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%lld", e->m_Value));

    if (e->m_color)
    {
        for (uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellTextColour(num_row, i, RGB_TO_WXCOLOR(*e->m_color));
    }

    if (e->m_bg_color)  /* Set custom color if it's given */
    {
        for (uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellBackgroundColour(num_row, i, RGB_TO_WXCOLOR(*e->m_bg_color));
    }
    else  /* Otherway use two colors alternately for all of the lines */
    {
        for (uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellBackgroundColour(num_row, i, (num_row & 1) ? 0xE6E6E6 : 0xFFFFFF);
    }

    if (e->m_is_bold || e->m_scale != 1.0 || !e->m_font_face.empty())
    {
        wxFont font;
        font.SetWeight(e->m_is_bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
        font.Scale(1.0f);  /* Scale has to be set to default first */

        for (uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellFont(num_row, i, font);

        font.Scale(e->m_scale);

        if (!e->m_font_face.empty())
            font.SetFaceName(e->m_font_face);

        for (uint8_t i = 0; i != ModbusGridCol::Modbus_Max; i++)
            m_grid->SetCellFont(num_row, i, font);
    }

    if (m_isReadOnly)
        m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, true);
}

void ModbusItemPanel::UpdatePanel()
{
    if(m_grid->GetNumberRows())
        m_grid->DeleteRows(0, m_grid->GetNumberRows());

    grid_to_entry.clear();

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    uint8_t default_fav_level = modbus_handler->GetFavouriteLevel();
    
    if (search_pattern.empty())
    {
        for (auto& e : m_items)
        {
            if (default_fav_level <= e->m_FavLevel)
            {
                AddItem(e);
            }
        }
    }
    else
    {
        for (auto& e : m_items)
        {
            if (default_fav_level <= e->m_FavLevel)
            {
                if (boost::icontains(e->m_Name, search_pattern))
                    AddItem(e);
            }
        }
    }
}

void ModbusItemPanel::UpdateChangesOnly(std::vector<uint8_t>& changed_rows)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

    //if (changed_rows.size() > m_items.size())
       // return;

    int last_row = 0xFF;
    int skipped_rows = 0;
    for(auto& i : changed_rows)
    {
        int num_row = m_grid->GetNumberRows() + skipped_rows;
        if (i == 0xFF) continue;

        if (i != 0xFF)
        {
            if (i + skipped_rows >= num_row)  /* was break */
                m_grid->AppendRows(1);
            else
                num_row = i + skipped_rows;
        }
        else
        {
            num_row = last_row + 1;
            skipped_rows++;
        }

       // m_grid->SetRowLabelValue(num_row, wxString::Format("%d", num_row));
        if (i != 0xFF)  /* Update value */
        {
            //m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), m_items[i]->m_Name);

            if (m_items[i]->m_Type == ModbusBitfieldType::MBT_FLOAT)
            {
                m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%.3f", m_items[i]->m_fValue));
            }
            else
            {
                if (m_items[i]->m_Format == ModbusValueFormat::MVF_DEC)
                {
                    m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%lld", m_items[i]->m_Value));
                }
                else if (m_items[i]->m_Format == ModbusValueFormat::MVF_HEX)
                {
                    m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%llx", m_items[i]->m_Value));
                }
                else if (m_items[i]->m_Format == ModbusValueFormat::MVF_BIN)
                {
                    m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), wxString::Format("%llx", m_items[i]->m_Value));  /* TODO */
                }
            }

            m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, m_isReadOnly);
        }
        else
        {
            m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Name, true);
            m_grid->SetReadOnly(num_row, ModbusGridCol::Modbus_Value, true);

            m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Name), "-");
            m_grid->SetCellValue(wxGridCellCoords(num_row, ModbusGridCol::Modbus_Value), "-");
        }
        last_row = num_row;
        m_grid->Update();
    }
}

ModbusDataPanel::ModbusDataPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    m_StyleEditDialog = new ModbusDataEditDialog(this);
    m_BitfieldEditor = new ModbusBitEditorDialog(this);

    if (modbus_handler->m_numEntries.coils)
        m_coil = new ModbusItemPanel(this, wxString::Format("Coil Status - %lld", modbus_handler->m_numEntries.coils), modbus_handler->m_coils, false);
    if (modbus_handler->m_numEntries.inputStatus)
        m_input = new ModbusItemPanel(this, wxString::Format("Input Status - %lld", modbus_handler->m_numEntries.inputStatus), modbus_handler->m_inputStatus, true);
    if (modbus_handler->m_numEntries.holdingRegisters)
        m_holding = new ModbusItemPanel(this, wxString::Format("Holding Registers - %lld", modbus_handler->m_numEntries.holdingRegisters), modbus_handler->m_Holding, false);
    if (modbus_handler->m_numEntries.Input)
        m_inputReg = new ModbusItemPanel(this, wxString::Format("Input Registers - %lld", modbus_handler->m_numEntries.Input), modbus_handler->m_Input, true);

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    m_hSizer = new wxBoxSizer(wxHORIZONTAL);

    if(modbus_handler->m_numEntries.coils)
        m_hSizer->Add(m_coil->static_box);
    if (modbus_handler->m_numEntries.inputStatus)
        m_hSizer->Add(m_input->static_box);
    if (modbus_handler->m_numEntries.holdingRegisters)
        m_hSizer->Add(m_holding->static_box);
    if (modbus_handler->m_numEntries.Input)
        m_hSizer->Add(m_inputReg->static_box);

    v_sizer->Add(m_hSizer);

    wxBoxSizer* h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    m_StartButton = new wxButton(this, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    m_StartButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;

            wxString tcp_ip = m_TcpIp->GetValue();
            uint16_t tcp_port = m_TcpPort->GetValue();
            uint16_t com_port = m_ComPort->GetValue();
            bool is_tcp = m_UseTcp->IsChecked();

            modbus_handler->GetSerial().SetTcpIp(tcp_ip.ToStdString());
            modbus_handler->GetSerial().SetTcpPort(tcp_port);
            modbus_handler->GetSerial().SetComPort(com_port);
            modbus_handler->GetSerial().SetTcp(is_tcp);
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

    h_sizer2->AddSpacer(10);
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

    h_sizer2->AddSpacer(10);
    h_sizer2->Add(new wxStaticText(this, wxID_ANY, "Response timeout [ms]: "));
    h_sizer2->AddSpacer(5);
    m_ResponseTimeout = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 50, 10000, modbus_handler->GetSerial().m_ResponseTimeout);
    m_ResponseTimeout->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, [this](wxSpinEvent& event)
        {
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            int new_value = event.GetValue();
            modbus_handler->GetSerial().m_ResponseTimeout = new_value;
        });

    h_sizer2->Add(m_ResponseTimeout);
    v_sizer->Add(h_sizer2);
    v_sizer->AddSpacer(5);

    wxBoxSizer* h_sizer3 = new wxBoxSizer(wxHORIZONTAL);
    h_sizer3->AddSpacer(235);
    h_sizer3->Add(new wxStaticText(this, wxID_ANY, "TCP IP:"));
    h_sizer3->AddSpacer(5);
    m_TcpIp = new wxTextCtrl(this, wxID_ANY, "");
    m_TcpIp->SetLabelText(modbus_handler->GetSerial().GetTcpIp());
    h_sizer3->Add(m_TcpIp);

    h_sizer3->AddSpacer(10);
    h_sizer3->Add(new wxStaticText(this, wxID_ANY, "TCP Port:"));
    h_sizer3->AddSpacer(5);
    m_TcpPort = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, modbus_handler->GetSerial().GetTcpPort());
    h_sizer3->Add(m_TcpPort);

    h_sizer3->AddSpacer(10);
    h_sizer3->Add(new wxStaticText(this, wxID_ANY, "COM Port:"));
    h_sizer3->AddSpacer(5);
    m_ComPort = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, modbus_handler->GetSerial().GetComPort());
    h_sizer3->Add(m_ComPort);

    h_sizer3->AddSpacer(10);
    h_sizer3->Add(new wxStaticText(this, wxID_ANY, "Use TCP?"));
    h_sizer3->AddSpacer(5);
    m_UseTcp = new wxCheckBox(this, wxID_ANY, "");
    m_UseTcp->SetValue(modbus_handler->GetSerial().IsTcp());
    h_sizer3->Add(m_UseTcp);

    v_sizer->Add(h_sizer3);

    wxBoxSizer* h_sizer4 = new wxBoxSizer(wxHORIZONTAL);
    m_ConnectionStatus = new wxStaticText(this, wxID_ANY, "Status: N/A");
    h_sizer4->Add(m_ConnectionStatus);
    v_sizer->Add(h_sizer4);

    SetSizer(v_sizer);
    Show();

}

void ModbusDataPanel::OnCellValueChanged(wxGridEvent& ev)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    int row = ev.GetRow(), col = ev.GetCol();
    if (row == -1 || col == -1)  /* Header */
        return;

    if(m_coil && ev.GetEventObject() == dynamic_cast<wxObject*>(m_coil->m_grid))
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
    else if(m_input && ev.GetEventObject() == dynamic_cast<wxObject*>(m_input->m_grid))
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
    else if(m_holding && ev.GetEventObject() == dynamic_cast<wxObject*>(m_holding->m_grid))
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
    else if(m_inputReg && ev.GetEventObject() == dynamic_cast<wxObject*>(m_inputReg->m_grid))
    {
        wxString new_value = m_inputReg->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case ModbusGridCol::Modbus_Name:
            {
                m_inputReg->m_items[row]->m_Name = new_value.ToStdString();
                break;
            }
        }
    }
}

void ModbusDataPanel::OnCellRightClick(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if (row == -1 || col == -1)  /* Header */
        return;

    ModbusItemPanel* item_panel = nullptr;
    ModbusItemType* item_list = nullptr;
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    if (m_coil && ev.GetEventObject() == dynamic_cast<wxObject*>(m_coil->m_grid))
    {
        item_panel = m_coil;
        item_list = &modbus_handler->m_coils;
    }
    else if (m_input && ev.GetEventObject() == dynamic_cast<wxObject*>(m_input->m_grid))
    {
        item_panel = m_input;
        item_list = &modbus_handler->m_inputStatus;
    }
    else if (m_holding && ev.GetEventObject() == dynamic_cast<wxObject*>(m_holding->m_grid))
    {
        item_panel = m_holding;
        item_list = &modbus_handler->m_Holding;
    }
    else if (m_inputReg && ev.GetEventObject() == dynamic_cast<wxObject*>(m_inputReg->m_grid))
    {
        item_panel = m_inputReg;
        item_list = &modbus_handler->m_Input;
    }

    if (item_panel != nullptr && item_list != nullptr)
    {
        wxMenu menu;
        if(item_panel == m_inputReg || item_panel == m_holding)
            menu.Append(ID_ModbusShowBits, "&Show Bits")->SetBitmap(wxArtProvider::GetBitmap(wxART_QUESTION, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_ModbusDataEdit, "&Edit")->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_ModbusDataDec, "&Dec")->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_ModbusDataHex, "&Hex")->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(14, 14))));
        menu.Append(ID_ModbusDataBin, "&Bin")->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(14, 14))));
        int ret = GetPopupMenuSelectionFromUser(menu);
        switch (ret)
        {
            case ID_ModbusShowBits:
            {
                bool to_exit = false;
                while (!to_exit)
                {
                    bool is_holding = item_panel == m_holding;
                    ModbusBitfieldInfo info = modbus_handler->GetMapForHolding(row, is_holding);
                    if (info.size() == 0)
                    {
                        wxMessageDialog(this, "There are no mapping found for selected CAN Frame", "Error", wxOK).ShowModal();
                        return;
                    }

                    m_BitfieldEditor->ShowDialog(info);
                    if (!is_holding)
                        break;

                    if ((m_BitfieldEditor->GetClickType() == ModbusBitEditorDialog::ClickType::Apply || m_BitfieldEditor->GetClickType() == ModbusBitEditorDialog::ClickType::Ok))
                    {
                        std::vector<std::string> ret = m_BitfieldEditor->GetOutput();
                        modbus_handler->ApplyEditingOnHolding(row, ret);
                        /*
                        std::string hex;
                        utils::ConvertHexBufferToString(can_grid_tx->grid_to_entry[row]->data, hex);
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_Data), wxString(hex));
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_DataSize),
                            wxString::Format("%lld", can_grid_tx->grid_to_entry[row]->data.size()));*/
                    }

                    if (m_BitfieldEditor->GetClickType() != ModbusBitEditorDialog::ClickType::Apply)
                    {
                        to_exit = true;
                        break;
                    }
                }
                break;
            }
            case ID_ModbusDataEdit:
            {
                std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                auto& item = item_list->at(row);
                m_StyleEditDialog->ShowDialog(item->m_color, item->m_bg_color, item->m_is_bold, item->m_font_face, item->m_scale);
                if (m_StyleEditDialog->IsApplyClicked())
                {
                    item->m_color = m_StyleEditDialog->GetTextColor();
                    item->m_bg_color = m_StyleEditDialog->GetBgColor();
                    item->m_is_bold = m_StyleEditDialog->IsBold();
                    item->m_scale = m_StyleEditDialog->GetScale();
                    item->m_font_face = m_StyleEditDialog->GetFontFace();

                    item_panel->UpdatePanel();
                }
                break;
            }
            case ID_ModbusDataDec:
            {
                std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                auto& item = item_list->at(row);

                item->m_Format = ModbusValueFormat::MVF_DEC;
                item_panel->UpdatePanel();
                break;
            }
            case ID_ModbusDataHex:
            {
                std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                auto& item = item_list->at(row);

                item->m_Format = ModbusValueFormat::MVF_HEX;
                item_panel->UpdatePanel();
                break;
            }
            case ID_ModbusDataBin:
            {
                std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                auto& item = item_list->at(row);

                item->m_Format = ModbusValueFormat::MVF_BIN;
                item_panel->UpdatePanel();
                break;
            }
        }
    }
}

void ModbusDataPanel::OnGridLabelLeftClick(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if (row == -1 || col == -1)  /* Header */
        return;

    wxGrid* grid = nullptr;
    ModbusItemType* item_list = nullptr;
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    if (m_coil && ev.GetEventObject() == dynamic_cast<wxObject*>(m_coil->m_grid))
    {
        grid = m_coil->m_grid;
        item_list = &modbus_handler->m_coils;
    }
    else if (m_input && ev.GetEventObject() == dynamic_cast<wxObject*>(m_input->m_grid))
    {
        grid = m_input->m_grid;
        item_list = &modbus_handler->m_inputStatus;
    }
    else if (m_holding && ev.GetEventObject() == dynamic_cast<wxObject*>(m_holding->m_grid))
    {
        grid = m_holding->m_grid;
        item_list = &modbus_handler->m_Holding;
    }
    else if (m_inputReg && ev.GetEventObject() == dynamic_cast<wxObject*>(m_inputReg->m_grid))
    {
        grid = m_inputReg->m_grid;
        item_list = &modbus_handler->m_Input;
    }

    if(grid != nullptr && item_list != nullptr)
    {
        wxPoint pos = wxGetMousePosition();
        wxWindow* w = wxFindWindowAtPoint(pos);
        auto& item = item_list->at(row);

        wxRect size(wxSize(100, 100));
        if (!item->m_Desc.empty())
        {
            tip = new wxTipWindow(grid, item->m_Desc, 100, &tip, &size);
            tip->Show();
        }
    }
    ev.Skip();
}

void ModbusDataPanel::OnGridLabelRightClick(wxGridEvent& ev)
{
    if (ev.GetEventObject() == dynamic_cast<wxObject*>(m_holding->m_grid))
    {
        wxMenu menu;
        menu.Append(ID_ModbusEditFavourites, "&Edit favourites")->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, FromDIP(wxSize(14, 14))));
        int ret = GetPopupMenuSelectionFromUser(menu);
        switch (ret)
        {
            case ID_ModbusEditFavourites:
            {
                std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
                wxTextEntryDialog d(this, "Enter default favourite level for modbus list", "Default favourite level");
                d.SetValue(std::to_string(modbus_handler->GetFavouriteLevel()));
                int ret = d.ShowModal();
                if (ret == wxID_OK)
                {
                    uint8_t favourite_level = 0;
                    try
                    {
                        favourite_level = std::stoi(d.GetValue().ToStdString());
                    }
                    catch (const std::exception& e)
                    {
                        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
                    }
                    modbus_handler->SetFavouriteLevel(favourite_level);

                    m_holding->UpdatePanel();
                }
                break;
            }
        }
    }
    ev.Skip();
}

void ModbusDataPanel::OnKeyDown(wxKeyEvent& evt)
{
    if (evt.ControlDown())
    {
        switch (evt.GetKeyCode())
        {
            case 'F':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if (focus == m_holding->m_grid)
                {
                    wxTextEntryDialog d(this, "Enter field name for what you want to filter", "Search for frame");
                    d.SetValue(m_holding->search_pattern);
                    int ret = d.ShowModal();
                    if (ret == wxID_OK)
                    {
                        m_holding->search_pattern = d.GetValue().ToStdString();
                        m_holding->UpdatePanel();
                    }
                    return;
                }
            }
        }
    }
    evt.Skip();
}

void ModbusDataPanel::OnSize(wxSizeEvent& event)
{
    event.Skip(true);
}

void ModbusDataPanel::On10MsTimer()
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    if (modbus_handler->GetSerial().IsOpen())
    {
        m_ConnectionStatus->SetLabelText("Status: Connected");
        m_ConnectionStatus->SetForegroundColour(*wxGREEN);
    }
    else
    {
        if (modbus_handler->GetSerial().IsErrorPresent())
        {
            m_ConnectionStatus->SetLabelText("Status: Error");
            m_ConnectionStatus->SetForegroundColour(*wxRED);
        }
        else
        {
            m_ConnectionStatus->SetLabelText("Status: Disconnected");
            m_ConnectionStatus->SetForegroundColour(*wxBLACK);
        }
    }
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

            if(!std::filesystem::exists("Modbus"))
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
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_ErrorType, "Err");
    m_grid->SetColLabelValue(ModbusLogGridCol::ModbusLog_Data, "Data");

    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Time, 50);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Direction, 30);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_FCode, 20);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_DataSize, 30);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_ErrorType, 70);
    m_grid->SetColSize(ModbusLogGridCol::ModbusLog_Data, 550);

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

ModbusSpecialRegisterPanel::ModbusSpecialRegisterPanel(wxWindow* parent) :
    wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    //modbus_handler->SetModbusHelper(this);

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
            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
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
    m_RecordingSave->SetToolTip("Save special recording to file");
    m_RecordingSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
#ifdef _WIN32
            const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

            if(!std::filesystem::exists("Modbus"))
                std::filesystem::create_directory("Modbus");
            std::string log_format = std::format("Modbus/ModbusSpecialLog_{:%Y.%m.%d_%H_%M_%OS}.csv", now);
            std::filesystem::path p(log_format);

            std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
            modbus_handler->SaveSpecialRecordingToFile(p);
#endif
        });
    h_sizer2->Add(m_RecordingSave);

    v_sizer->Add(h_sizer2);

    static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Log :: TX: 0, RX: 0, Err: 0 - Total: 0");
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(*wxBLUE);

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600), 0);

    // Grid
    m_grid->CreateGrid(1, ModbusSpecialRegisterCol::ModbusSpec_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(ModbusSpecialRegisterCol::ModbusSpec_Time, "Time");
    m_grid->SetColLabelValue(ModbusSpecialRegisterCol::ModbusSpec_DataHex, "Hex");
    m_grid->SetColLabelValue(ModbusSpecialRegisterCol::ModbusSpec_Data, "Data");

    m_grid->SetColSize(ModbusSpecialRegisterCol::ModbusSpec_Time, 50);
    m_grid->SetColSize(ModbusSpecialRegisterCol::ModbusSpec_DataHex, 300);
    m_grid->SetColSize(ModbusSpecialRegisterCol::ModbusSpec_Data, 300);

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    m_grid->SetRowLabelSize(40);
    m_grid->GetGridWindow()->SetDoubleBuffered(true);

    static_box->Add(m_grid);
    v_sizer->Add(static_box, wxSizerFlags(1).Top().Expand());

    SetSizer(v_sizer);
    Show();
}

void ModbusSpecialRegisterPanel::On10MsTimer()
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    if(!modbus_handler->m_EventLogEntries.empty())
    {
        if(m_grid->GetNumberRows() > modbus_handler->m_EventLogEntries.size())
        {
            ClearRecordingsFromGrid();
        }

        if(!is_something_inserted)
            inserted_until = 0;

        auto it = modbus_handler->m_EventLogEntries.begin();
        std::advance(it, inserted_until);
        if(it == modbus_handler->m_EventLogEntries.end())
        {
            //DBG("shit happend");
            return;
        }

        if(it != modbus_handler->m_EventLogEntries.end())
        {
            for(; it != modbus_handler->m_EventLogEntries.end(); ++it)
            {
                AppendLog((*it)->last_execution, (*it)->data);
                inserted_until++;
            }
            //inserted_until = std::distance(can_handler->m_LogEntries.begin(), it);
            is_something_inserted = true;
        }
    }

}

void ModbusSpecialRegisterPanel::OnKeyDown(wxKeyEvent& evt)
{

}

void ModbusSpecialRegisterPanel::AppendLog(std::chrono::steady_clock::time_point t1, const std::vector<uint16_t>& data)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - modbus_handler->GetStartTime()).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusSpecialRegisterCol::ModbusSpec_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusSpecialRegisterCol::ModbusSpec_DataHex), hex);

    uint16_t error_id = data[8];
    std::string data_str;
    uint32_t time_time = data[1] | data[0] << 16;
    uint32_t time_date = data[3] | data[2] << 16;

    int hour = time_time / 10000;
    int minute = (time_time / 100) % 100;
    int second = time_time % 100;

    int year = time_date % 100;
    int month = (time_date / 100) % 100;
    int day = time_date / 10000;

    data_str += std::format("{:04}.{:02}.{:02} {:02}:{:02}:{:02} - ", 2000 + year, month, day, hour, minute, second);
    switch(error_id)
    {
        case 100:
        {
            data_str += std::format("SpecialError1");
            break;
        }
        default:
        {
            data_str += std::format("Error: {} - {}, {}, {}, {}", error_id, data[4], data[5], data[6], data[7]);
            break;
        }
    }
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusSpecialRegisterCol::ModbusSpec_Data), data_str);

    for(uint8_t i = 0; i != ModbusSpecialRegisterCol::ModbusSpec_Max; i++)
    {
        m_grid->SetCellBackgroundColour(cnt, i, (cnt % 2) == 0 ? 0xE6E6E6 : 0xFFFFFF);
    }

    m_grid->Update();

    if(m_AutoScroll)
        m_grid->ScrollLines(num_rows);

    cnt++;
}

void ModbusSpecialRegisterPanel::ClearRecordingsFromGrid()
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows)
        m_grid->DeleteRows(0, num_rows);
    cnt = 0;

    is_something_inserted = false;
    inserted_until = 0;
}

void ModbusSpecialRegisterPanel::OnSize(wxSizeEvent& event)
{
    event.Skip(true);
}

void ModbusLogPanel::AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, uint8_t error, const std::vector<uint8_t>& data)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - modbus_handler->GetStartTime()).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Direction), direction == CAN_LOG_DIR_TX ? "TX" : "RX");
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_FCode), wxString::Format("%X", fcode));

    wxString error_type;
    switch (error)
    {
        case ModbusErorrType::MB_ERR_CRC:
        {
            error_type = "CRC";
            break;
        }
        case ModbusErorrType::MB_ERR_TIMEOUT:
        {
            error_type = "TO";
            break;
        }
        case ModbusErorrType::MB_ERR_ILLEGAL_FUNCTION:
        {
            error_type = "MB_ERR_ILLEGAL_FUNCTION";
            break;
        }
        case ModbusErorrType::MB_ERR_ILLEGAL_DATA_ADDRESS:
        {
            error_type = "MB_ERR_ILLEGAL_DATA_ADDRESS";
            break;
        }
        case ModbusErorrType::MB_ERR_ILLEGAL_DATA_VALUE:
        {
            error_type = "MB_ERR_ILLEGAL_DATA_VALUE";
            break;
        }
        case ModbusErorrType::MB_ERR_SLAVE_DEVICE_FAILURE:
        {
            error_type = "MB_ERR_SLAVE_DEVICE_FAILURE";
            break;
        }
        case ModbusErorrType::MB_ERR_ACK:
        {
            error_type = "MB_ERR_ACK";
            break;
        }
        case ModbusErorrType::MB_ERR_SLAVE_DEVICE_BUSY:
        {
            error_type = "MB_ERR_SLAVE_DEVICE_BUSY";
            break;
        }
        case ModbusErorrType::MB_ERR_NAK:
        {
            error_type = "MB_ERR_NAK";
            break;
        }
        case ModbusErorrType::MB_ERR_MEMORY_PARITY_ERROR:
        {
            error_type = "MB_ERR_MEMORY_PARITY_ERROR";
            break;
        }
        case ModbusErorrType::MB_ERR_GATEWAY_UNAVAILABLE:
        {
            error_type = "MB_ERR_GATEWAY_UNAVAILABLE";
            break;
        }
        case ModbusErorrType::MB_ERR_GATEWAY_TARGET_FAILED:
        {
            error_type = "MB_ERR_GATEWAY_TARGET_FAILED";
            break;
        }
    }
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_Data), hex + " " + error_type);

    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_ErrorType), error_type);
    m_grid->SetCellValue(wxGridCellCoords(cnt, ModbusLogGridCol::ModbusLog_DataSize), wxString::Format("%lld", data.size()));

    if(m_AutoScroll)
        m_grid->ScrollLines(num_rows);

    for(uint8_t i = 0; i != ModbusLogGridCol::ModbusLog_Max; i++)
    {
        m_grid->SetReadOnly(cnt, i, true);

        if(error == MB_ERR_OK)
            m_grid->SetCellBackgroundColour(cnt, i, (direction == CAN_LOG_DIR_RX) ? 0xE6E6E6 : 0xFFFFFF);
        else
            m_grid->SetCellBackgroundColour(cnt, i, *wxRED);
    }

    cnt++;
}

void ModbusLogPanel::OnMaxEntriesReached()
{
    ClearRecordingsFromGrid();
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
        if (m_grid->GetNumberRows() > modbus_handler->m_LogEntries.size())
        {
            ClearRecordingsFromGrid();
        }

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
                AppendLog((*it)->last_execution, (*it)->direction, (*it)->fcode, (*it)->error_type, (*it)->data);
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

void ModbusLogPanel::OnKeyDown(wxKeyEvent& evt)
{
    if (evt.ControlDown())
    {
        switch (evt.GetKeyCode())
        {
            case 'C':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if (focus == m_grid)
                {
                    wxArrayInt rows = m_grid->GetSelectedRows();
                    if (rows.empty()) return;

                    wxString str_to_copy;
                    for (auto& row : rows)
                    {
                        for (uint8_t col = 0; col < ModbusLogGridCol::ModbusLog_Max; col++)
                        {
                            str_to_copy += m_grid->GetCellValue(row, col);
                            str_to_copy += '\t';
                        }
                        str_to_copy += '\n';
                    }
                    if (str_to_copy.Last() == '\n')
                        str_to_copy.RemoveLast();

                    if (wxTheClipboard->Open())
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

ModbusMasterPanel::ModbusMasterPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
    wxSize client_size = GetClientSize();

    m_mgr.SetManagedWindow(this);
	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    data_panel = new ModbusDataPanel(this);
    log_panel = new ModbusLogPanel(this);
    special_panel = new ModbusSpecialRegisterPanel(this);

    m_notebook->Freeze();
    m_notebook->AddPage(data_panel, "Data", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(log_panel, "Log", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(special_panel, "Special", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ModbusMasterPanel::Changeing), NULL, this);
    m_notebook->Split(0, wxLEFT);
    //m_notebook->Split(1, wxDOWN);
    //m_notebook->Split(2, wxDOWN);
    m_notebook->Thaw();
}

ModbusMasterPanel::~ModbusMasterPanel()
{
    m_mgr.UnInit();
}

void ModbusMasterPanel::UpdateSubpanels()
{

}

void ModbusMasterPanel::On10MsTimer()
{
    std::unique_ptr<ModbusEntryHandler>& modbus_handler = wxGetApp().modbus_handler;
    std::scoped_lock lock{ modbus_handler->m };

    if(data_panel)
        data_panel->On10MsTimer();
    if(log_panel)
        log_panel->On10MsTimer();
    if(special_panel)
        special_panel->On10MsTimer();
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

ModbusDataEditDialog::ModbusDataEditDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Modbus Style editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

    wxSizer* const sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&CAN style properties");
    {
        m_useCustomColor = new wxCheckBox(this, wxID_ANY, "Use custom color?");
        sizerMsgs->Add(m_useCustomColor);

        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Color:"));
        m_color = new wxColourPickerCtrl(this, wxID_ANY);
        sizerMsgs->Add(m_color);
    }

    {
        m_useCustomBackgroundColor = new wxCheckBox(this, wxID_ANY, "Use custom background color?");
        sizerMsgs->Add(m_useCustomBackgroundColor);

        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Background color:"));
        m_backgroundColor = new wxColourPickerCtrl(this, wxID_ANY);
        sizerMsgs->Add(m_backgroundColor);
    }

    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Bold?"));
        m_isBold = new wxCheckBox(this, wxID_ANY, "");
        sizerMsgs->Add(m_isBold);
    }

    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Font Type:"));
        m_fontFace = new wxFontPickerCtrl(this, wxID_ANY);
        sizerMsgs->Add(m_fontFace);
    }

    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Scale:"));
        m_scale = new wxSpinCtrlDouble(this, wxID_ANY, "0.0", wxDefaultPosition, wxDefaultSize, 16384, 0.0, 10.0, 1.0, 0.2);
        sizerMsgs->Add(m_scale);
    }

    sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

    SetSizerAndFit(sizerTop);
    CentreOnScreen();
}

void ModbusDataEditDialog::ShowDialog(std::optional<uint32_t> color, std::optional<uint32_t> bg_color, bool is_bold, const wxString& font_face, float scale)
{
    uint32_t color_rgb = color.has_value() ? *color : 0xFFFFFF;
    uint32_t bg_color_rgb = bg_color.has_value() ? *bg_color : 0xFFFFFF;

    m_useCustomColor->SetValue(color.has_value());
    m_color->SetColour(RGB_TO_WXCOLOR(color_rgb));
    m_backgroundColor->SetColour(RGB_TO_WXCOLOR(bg_color_rgb));
    m_useCustomBackgroundColor->SetValue(bg_color.has_value());
    m_isBold->SetValue(is_bold);

    if (!font_face.empty())
    {
        wxFont f;
        f.SetFaceName(font_face);
        m_fontFace->SetSelectedFont(f);
    }
    m_scale->SetValue(static_cast<double>(scale));

    m_IsApplyClicked = false;
    ShowModal();
    DBG("isapply: %d", IsApplyClicked());
}

std::optional<uint32_t> ModbusDataEditDialog::GetTextColor()
{
    std::optional<uint32_t> ret;
    if (m_useCustomColor->IsChecked())
    {
        uint32_t color = m_color->GetColour().GetRGB();
        ret = WXCOLOR_TO_RGB(color);
    }
    return ret;
}

std::optional<uint32_t> ModbusDataEditDialog::GetBgColor()
{
    std::optional<uint32_t> ret;
    if (m_useCustomBackgroundColor->IsChecked())
    {
        uint32_t color = m_backgroundColor->GetColour().GetRGB();
        ret = WXCOLOR_TO_RGB(color);
    }
    return ret;
}

void ModbusDataEditDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    Close();
    m_IsApplyClicked = true;
}

ModbusBitEditorDialog::ModbusBitEditorDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Bit editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);
    sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Bit editor");

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_IsDecimal = new wxRadioButton(this, wxID_ANY, "Decimal");
    m_IsDecimal->Bind(wxEVT_RADIOBUTTON, &ModbusBitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsDecimal);
    m_IsHex = new wxRadioButton(this, wxID_ANY, "Hex");
    m_IsHex->Bind(wxEVT_RADIOBUTTON, &ModbusBitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsHex);
    m_IsBinary = new wxRadioButton(this, wxID_ANY, "Binary");
    m_IsBinary->Bind(wxEVT_RADIOBUTTON, &ModbusBitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsBinary);

    sizerMsgs->Add(h_sizer);
    sizerMsgs->AddSpacer(20);

    for (int i = 0; i != MAX_BITEDITOR_FIELDS; i++)
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

void ModbusBitEditorDialog::ShowDialog(ModbusBitfieldInfo& values)
{
    if (values.size() > MAX_BITEDITOR_FIELDS)
    {
        values.resize(MAX_BITEDITOR_FIELDS);
        //LOG(LogLevel::Warning, "Too much bitfields used for can frame mapping. FrameID: {:X}, Used: {}, Maximum supported: {}", frame_id, values.size(), MAX_BITEDITOR_FIELDS);
    }

    m_Id = 0;
    m_DataFormat = 0;
    m_BitfieldInfo = values;

    for (const auto& [label, value, frame] : values)
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

    for (int i = m_Id; i != MAX_BITEDITOR_FIELDS; i++)
    {
        m_InputLabel[i]->Hide();
        m_InputLabel[i]->SetToolTip("");
        m_Input[i]->Hide();
    }

    SetTitle(wxString::Format("Bit editor"));
    bit_sel = BitSelection::Decimal;
    m_IsDecimal->SetValue(true);
    m_IsHex->SetValue(false);
    m_IsBinary->SetValue(false);

    sizerTop->Layout();
    sizerTop->Fit(this);

    m_ClickType = ClickType::None;
    int ret = ShowModal();
    DBG("ShowModal ret: %d\n", ret);
}

std::vector<std::string> ModbusBitEditorDialog::GetOutput()
{
    std::vector<std::string> ret;
    for (int i = 0; i != m_Id; i++)
    {
        std::string input_ret = m_Input[i]->GetValue().ToStdString();
        switch (bit_sel)
        {
        case BitSelection::Hex:
        {
            try
            {
                input_ret = std::to_string(std::stoll(input_ret, nullptr, 16));
            }
            catch (...)
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
            catch (...)
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

void ModbusBitEditorDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnApply %d\n", (int)m_ClickType)
        EndModal(wxID_APPLY);
    //Close();
    m_ClickType = ClickType::Apply;
}

void ModbusBitEditorDialog::OnOk(wxCommandEvent& event)
{
    DBG("OnOK %d\n", (int)m_ClickType);
    if (m_ClickType == ClickType::Close)
        return;
    EndModal(wxID_OK);
    //Close();
    m_ClickType = ClickType::Ok;
    event.Skip();
}

void ModbusBitEditorDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnCancel %d\n", (int)m_ClickType);
    EndModal(wxID_CLOSE);
    m_ClickType = ClickType::Close;
    //Close();
}

void ModbusBitEditorDialog::OnClose(wxCloseEvent& event)
{
    DBG("OnClose %d\n", (int)m_ClickType);
    m_ClickType = ClickType::Close;
    EndModal(wxID_CLOSE);
}

void ModbusBitEditorDialog::OnRadioButtonClicked(wxCommandEvent& event)
{
    if (event.GetEventObject() == dynamic_cast<wxObject*>(m_IsDecimal))
    {
        uint8_t cnt = 0;
        for (const auto& [label, value, frame] : m_BitfieldInfo)
        {
            m_Input[cnt]->SetValue(value);
            if (++cnt > m_Id)
                break;
        }
        bit_sel = BitSelection::Decimal;
    }
    else if (event.GetEventObject() == dynamic_cast<wxObject*>(m_IsHex))
    {
        uint8_t cnt = 0;
        for (const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if (utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:X}", decimal_val);  /* std::to_chars gives lower case letters, I don't like it :/ */

                m_Input[cnt]->SetValue(hex_str);
                if (++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Hex;
    }
    else if (event.GetEventObject() == dynamic_cast<wxObject*>(m_IsBinary))
    {
        uint8_t cnt = 0;
        for (const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if (utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:b}", decimal_val);

                m_Input[cnt]->SetValue(hex_str);
                if (++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Binary;
    }
}
