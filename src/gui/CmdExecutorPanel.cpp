#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CmdExecutorPanelBase, wxPanel)
EVT_SIZE(CmdExecutorPanelBase::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(CmdExecutorPanelPage, wxPanel)
EVT_SIZE(CmdExecutorPanelPage::OnSize)
wxEND_EVENT_TABLE()

constexpr size_t MAX_CMD_LEN_FOR_BUTTON = 16;

/* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
#define RGB_TO_WXCOLOR(color) \
    wxColour(boost::endian::endian_reverse(color << 8))

#define WXCOLOR_TO_RGB(color) \
    boost::endian::endian_reverse(color << 8)

CmdExecutorEditDialog::CmdExecutorEditDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Command editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);


    wxSizer* const sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Command properties");
    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Name:"));
        m_commandName = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(350, 25), 0);
        sizerMsgs->Add(m_commandName);
    }
    
    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Command line:"));
        m_cmdToExecute = new wxTextCtrl(this, wxID_ANY, "a", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        sizerMsgs->Add(m_cmdToExecute, wxSizerFlags(1).Expand().Border(wxBOTTOM));
    }
    
    {
        sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Color:"));
        m_color = new wxColourPickerCtrl(this, wxID_ANY);
        sizerMsgs->Add(m_color);
    }

    {
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

void CmdExecutorEditDialog::ShowDialog(const wxString& cmd_name, const wxString& cmd_to_execute, uint32_t color, uint32_t bg_color, bool is_bold, float scale)
{
    m_commandName->SetLabel(cmd_name);
    m_cmdToExecute->SetLabel(cmd_to_execute);
    m_color->SetColour(RGB_TO_WXCOLOR(color));
    m_backgroundColor->SetColour(RGB_TO_WXCOLOR(bg_color));
    m_isBold->SetValue(is_bold);
    m_scale->SetValue(static_cast<double>(scale));

    m_IsApplyClicked = false;
    ShowModal();
    DBG("isapply: %d", IsApplyClicked());
}

void CmdExecutorEditDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    Close();
    m_IsApplyClicked = true;
}

wxBEGIN_EVENT_TABLE(CmdExecutorEditDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, CmdExecutorEditDialog::OnApply)
wxEND_EVENT_TABLE()

CmdExecutorPanelBase::CmdExecutorPanelBase(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{
    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    CmdExecutor* cmd = wxGetApp().cmd_executor;
    cmd->SetMediator(this);
    ReloadCommands();
    Show();
}

void CmdExecutorPanelBase::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

CmdExecutorPanelPage::CmdExecutorPanelPage(wxWindow* parent, uint8_t id, uint8_t cols)
    : wxPanel(parent, wxID_ANY), m_Id(id)
{
    edit_dlg = new CmdExecutorEditDialog(this);

    DBG("CmdExecutorPanelPage constructor %d, %d\n", id, cols);
    Bind(wxEVT_RIGHT_DOWN, &CmdExecutorPanelPage::OnPanelRightClick, this);
}

void CmdExecutorPanelPage::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CmdExecutorPanelPage::ToggleAllButtonClickability(bool toggle)
{
    for(auto& i : m_ButtonMap)
    {
        std::visit([toggle](auto& btn)
            {
                using T = std::decay_t<decltype(btn)>;
                if constexpr(std::is_same_v<T, wxButton*>)
                {
                    if(btn)
                        btn->Enable(toggle);
                }
            }, i.second);
    }
}

void CmdExecutorPanelPage::OnPanelRightClick(wxMouseEvent& event)
{
    wxMenu menu;
    menu.Append(ID_CmdExecutorAdd, "&Add")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));

    const wxPoint pt = wxGetMousePosition();
    int mouseX = pt.x - this->GetScreenPosition().x;
    int mouseY = pt.y - this->GetScreenPosition().y;

    std::vector<wxSize> pos;
    uint8_t col = 1;
    uint8_t curr_sizer = 0xFF;
    for(auto& i : m_VertialBoxes)
    {
        wxSize sizer_pos_min = i->GetSize();
        if(col == 1)
            sizer_pos_min = wxSize(0, 0);
        else
            sizer_pos_min.x = sizer_pos_min.x * col;  

        wxSize sizer_pos_max = i->GetSize();
        if(col == 1)
            sizer_pos_max.x = sizer_pos_max.x * col;
        else
            sizer_pos_max.x = sizer_pos_max.x * (2 * col);

        DBG("mouse pos: %d, %d\n", mouseX, mouseY);
        DBG("sizer size: %d, %d\n", i->GetSize().x, i->GetSize().y);
        DBG("sizer min: %d\n", (i->GetSize().x * (col - 1)));
        DBG("sizer max: %d\n", (i->GetSize().x * col));
        DBG("sizer pos: %d\n\n", (i->GetPosition().x * col));
        if(mouseX > sizer_pos_min.x && mouseX < sizer_pos_max.x)
        {
            curr_sizer = col - 1;
                break;
        }
        col++;
    }

    if(col == m_VertialBoxes.size() + 1)
    {
        DBG("invalid item");
        return;
    }

    int ret = GetPopupMenuSelectionFromUser(menu);
    switch(ret)
    {
        case ID_CmdExecutorAdd:
        {           
            CmdExecutor* cmd = wxGetApp().cmd_executor;
            cmd->AddCommand(m_Id, col, Command("New cmd", "& ping 127.0.0.1 -n 3 > nul", 0x33FF33, 0xFFFFFF, false, 1.0f));
            m_BaseGrid->Layout();
            break;
        }
    }
}

void CmdExecutorPanelPage::OnClick(wxCommandEvent& event)
{
    auto obj = event.GetEventObject();

    wxButton* btn = dynamic_cast<wxButton*>(obj);
    if(btn == nullptr)
    {
        LOG(LogLevel::Error, "btn is nullptr");
        return;
    }

    void* clientdata = btn->GetClientData();
    if(clientdata == nullptr)
    {
        LOG(LogLevel::Error, "clientdata is nullptr");
        return;
    }

    Command* c = reinterpret_cast<Command*>(clientdata);
    ToggleAllButtonClickability(false);
    c->Execute();
    ToggleAllButtonClickability(true);
}

void CmdExecutorPanelPage::OnRightClick(wxMouseEvent& event)
{
    auto obj = event.GetEventObject();

    wxButton* btn = dynamic_cast<wxButton*>(obj);
    if(btn == nullptr)
    {
        LOG(LogLevel::Error, "btn is nullptr");
        return;
    }

    void* clientdata = btn->GetClientData();
    if(clientdata == nullptr)
    {
        LOG(LogLevel::Error, "clientdata is nullptr");
        return;
    }

    Command* c = reinterpret_cast<Command*>(clientdata);
    DBG("rightclick");

    wxMenu menu;
    menu.Append(ID_CmdExecutorEdit, "&Edit")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDelete, "&Delete")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    int ret = GetPopupMenuSelectionFromUser(menu);
    
    switch(ret)
    {
        case ID_CmdExecutorEdit:
        {
            edit_dlg->ShowDialog(c->GetName(), c->GetCmd(), c->GetColor(), c->GetBackgroundColor(), c->IsBold(), c->GetScale());
            if(!edit_dlg->IsApplyClicked()) return;

            const std::string cmd_name = edit_dlg->GetCmdName().ToStdString();
            const std::string cmd = edit_dlg->GetCmd().ToStdString();
            wxColor color = edit_dlg->GetTextColor();
            wxColor bg_color = edit_dlg->GetBgColor();
            bool is_bold = edit_dlg->IsBold();
            float scale = edit_dlg->GetScale();

            c->SetName(cmd_name).SetCmd(cmd).SetColor(WXCOLOR_TO_RGB(color.GetRGB())).SetBackgroundColor(WXCOLOR_TO_RGB(bg_color.GetRGB())).SetBold(is_bold).SetScale(scale);

            UpdateCommandButon(c, btn, true);
            break;
        }
        case ID_CmdExecutorDelete:
        {
            wxMessageDialog d(this, "Are you sure want to delete this item?", "Error", wxOK | wxCANCEL);
            int ret_code = d.ShowModal();
            if(ret_code == wxID_OK)  /* OK */
            {
                DeleteCommandButton(nullptr, btn);
            }
            break;
        }
    }
}

void CmdExecutorPanelBase::ReloadCommands()
{
    CmdExecutor* cmd = wxGetApp().cmd_executor;
    cmd->ReloadCommandsFromFile();
}

void CmdExecutorPanelBase::OnPreReload(uint8_t page)
{
    DBG("OnPreReload pages: %d\n", page);

    m_notebook->DeleteAllPages();
    m_Pages.clear();

    m_notebook->Freeze();
    for(uint8_t i = 0; i != page; i++)
    {
        CmdExecutorPanelPage* p = new CmdExecutorPanelPage(m_notebook, i + 1, 0);
        m_notebook->AddPage(p, std::format("Page {}", i + 1), false, wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_OTHER, FromDIP(wxSize(16, 16))));
        m_Pages.push_back(p);
    }
    m_notebook->Thaw();
}

void CmdExecutorPanelBase::OnPreReloadColumns(uint8_t pages, uint8_t cols)
{
    m_Pages[pages - 1]->OnPreload(cols);
    DBG("OnPreReloadColumns pages: %d, cols: %d\n", pages, cols);
}

void CmdExecutorPanelBase::OnCommandLoaded(uint8_t page, uint8_t col, CommandTypes cmd)
{
    m_Pages[page - 1]->OnCommandLoaded(col, cmd);
    /*
    for(auto& i : m_Pages)
    {
        if(i->m_Id == page)
        {
            i->OnCommandLoaded(col, cmd);
            break;
        }
    }*/
}

void CmdExecutorPanelPage::OnCommandLoaded(uint8_t col, CommandTypes cmd)
{
    std::visit([this, col](auto& concrete_cmd)
        {
            using T = std::decay_t<decltype(concrete_cmd)>;
            if constexpr(std::is_same_v<T, std::shared_ptr<Command>>)
            {
                AddCommandElement(col, concrete_cmd.get());
            }
            else if constexpr(std::is_same_v<T, Separator>)
            {
                AddSeparatorElement(col, concrete_cmd);
            }
            else
                static_assert(always_false_v<T>, "CmdExecutorPanel::OnCommandLoaded Bad visitor!");
        }, cmd);
}

void CmdExecutorPanelBase::OnPostReload(uint8_t page, uint8_t cols, CommandPageNames& names)
{
    m_Pages[page - 1]->OnPostReloadUpdate();

    m_notebook->Freeze();
    m_notebook->SetPageText(page - 1, names.back());
    m_notebook->Thaw();
}

void CmdExecutorPanelPage::OnPreload(uint8_t cols)
{
    if(m_BaseGrid != nullptr)
    {
        m_BaseGrid->Clear(true);
        m_VertialBoxes.clear();
        m_ButtonMap.clear();
    }

    m_BaseGrid = new wxGridSizer(cols);
    m_BaseGrid->Layout();
    for(uint8_t i = 0; i != cols; i++)
    {
        wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
        m_BaseGrid->Add(box_sizer);
        m_VertialBoxes.push_back(box_sizer);
    }
    m_BaseGrid->Layout();
}

void CmdExecutorPanelPage::OnPostReloadUpdate()
{
    wxSize old_size = GetSize();
    for(auto& i : m_VertialBoxes)
    {
        i->Layout();
    }
    m_BaseGrid->Layout();
    SetSizerAndFit(m_BaseGrid);
    SetSize(old_size);  /* Size has to be set, because if isn't, only the first column will appear in the base grid after reloading */
}

void CmdExecutorPanelPage::AddCommandElement(uint8_t col, Command* c)
{
    wxButton* btn = new wxButton(this, wxID_ANY, !c->GetName().empty() ? c->GetName() : c->GetCmd().substr(0, MAX_CMD_LEN_FOR_BUTTON), wxDefaultPosition, wxDefaultSize);
    UpdateCommandButon(c, btn);

    btn->SetClientData((void*)c);
    btn->Bind(wxEVT_BUTTON, &CmdExecutorPanelPage::OnClick, this);
    btn->Bind(wxEVT_RIGHT_DOWN, &CmdExecutorPanelPage::OnRightClick, this);

    m_ButtonMap.emplace(col - 1, btn);
    m_VertialBoxes[col - 1]->Add(btn);
}

void CmdExecutorPanelPage::AddSeparatorElement(uint8_t col, Separator s)
{
    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, s.width), wxLI_HORIZONTAL);
    m_ButtonMap.emplace(col - 1, line);
    m_VertialBoxes[col - 1]->Add(line);

    line->Bind(wxEVT_RIGHT_DOWN, &CmdExecutorPanelPage::OnRightClick, this);
}

void CmdExecutorPanelPage::UpdateCommandButon(Command* c, wxButton* btn, bool force_font_reset)
{
    btn->SetToolTip(c->GetCmd());
    btn->SetForegroundColour(RGB_TO_WXCOLOR(c->GetColor()));  /* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
    btn->SetBackgroundColour(RGB_TO_WXCOLOR(c->GetBackgroundColor()));

    wxFont font;
    font.SetWeight(c->IsBold() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
    font.Scale(1.0f);  /* Scale has to be set to default first */
    btn->SetFont(font);
    font.Scale(c->GetScale());
    btn->SetFont(font);

    if(force_font_reset)
    {
        std::string new_name = !c->GetName().empty() ? c->GetName() : c->GetCmd().substr(0, MAX_CMD_LEN_FOR_BUTTON);
        btn->SetLabelText(new_name);
    }
    m_BaseGrid->Layout();
}

void CmdExecutorPanelPage::DeleteCommandButton(Command* c, wxButton* btn)
{
    auto it = m_ButtonMap.begin();
    while(it != m_ButtonMap.end())
    {
         bool ret = std::visit([btn, &it, this](auto& it_button)
            {
                using T = std::decay_t<decltype(it_button)>;
                if constexpr(std::is_same_v<T, wxButton*>)
                {
                    if(it_button == btn)
                    {
                        it = m_ButtonMap.erase(it);
                        return true;
                    }
                }
                return false;
            }, it->second);

        if(ret)
            break;
        else
            ++it;
    }

    //btn->RemoveChild(this);
    btn->DeletePendingEvents();
    btn->Disconnect(wxEVT_BUTTON);
    btn->Disconnect(wxEVT_RIGHT_DOWN);
    btn->Destroy();

    m_BaseGrid->Layout();
}