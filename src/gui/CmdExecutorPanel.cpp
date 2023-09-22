#include "pch.hpp"
#include <wx/bmpcbox.h>

wxBEGIN_EVENT_TABLE(CmdExecutorPanelBase, wxPanel)
EVT_SIZE(CmdExecutorPanelBase::OnSize)
EVT_MIDDLE_DOWN(CmdExecutorPanelBase::OnMiddleClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(CmdExecutorPanelPage, wxPanel)
EVT_SIZE(CmdExecutorPanelPage::OnSize)
//EVT_PAINT(CmdExecutorPanelPage::OnPaint)
wxEND_EVENT_TABLE()

constexpr size_t MAX_CMD_LEN_FOR_BUTTON = 16;

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

void CmdExecutorEditDialog::ShowDialog(const wxString& cmd_name, const wxString& cmd_to_execute, uint32_t color, uint32_t bg_color, bool is_bold, const wxString& font_face, float scale)
{
    m_commandName->SetLabel(cmd_name);
    m_cmdToExecute->SetLabel(cmd_to_execute);
    m_color->SetColour(RGB_TO_WXCOLOR(color));
    m_backgroundColor->SetColour(RGB_TO_WXCOLOR(bg_color));
    m_isBold->SetValue(is_bold);

    if(!font_face.empty())
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
    m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN, wxAuiNotebookEventHandler(CmdExecutorPanelBase::OnAuiRightClick), NULL, this);

    std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
    cmd->SetMediator(this);
    ReloadCommands();
    m_notebook->Layout();
    Show();

    Bind(wxEVT_RIGHT_DOWN, &CmdExecutorPanelBase::OnPanelRightClick, this);
}


void CmdExecutorPanelBase::OnPanelRightClick(wxMouseEvent& event)
{
    DBG("click");
    event.Skip();
}

void CmdExecutorPanelBase::ReloadCommands()
{
    std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
    cmd->ReloadCommandsFromFile();
}

void CmdExecutorPanelBase::OnSize(wxSizeEvent& evt)
{
    wxSize new_size = evt.GetSize();
    if(m_notebook)
        m_notebook->SetSize(new_size);
    evt.Skip(true);
}

void CmdExecutorPanelBase::OnAuiRightClick(wxAuiNotebookEvent& evt)
{
    int page_id = m_CurrentPage = evt.GetSelection();

    wxMenu menu;
    menu.Append(ID_CmdExecutorEditPageName, "&Rename")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorChangeIcon, "&Change icon")->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorAddPage, "&Add")->SetBitmap(wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDeletePage, "&Delete")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDuplicatePageBefore, "&Duplicate Before")->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDuplicatePageAfter, "&Duplicate After")->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, FromDIP(wxSize(14, 14))));
    int ret = GetPopupMenuSelectionFromUser(menu);

    switch(ret)
    {
        case ID_CmdExecutorEditPageName:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            CommandPageNames& page_names = cmd->GetPageNames();

            wxTextEntryDialog d(this, "Type new page name here", "Rename");
            d.SetValue(page_names[page_id]);
            int ret = d.ShowModal();
            if(ret == wxID_OK)
            {
                m_notebook->Freeze();
                m_notebook->SetPageText(page_id, d.GetValue());
                m_notebook->Thaw();

                page_names[page_id] = d.GetValue().ToStdString();
            }
            break;
        }
        case ID_CmdExecutorChangeIcon:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            CommandPageIcons& page_icons = cmd->GetPageIcons();

            IconSelectionDialog d(this);
            d.SelectIconByName(page_icons[page_id]);
            if(d.ShowModal() == wxID_OK)
            {
                wxString icon_name = d.GetSelectedIcon();
                m_notebook->Freeze();
                m_notebook->SetPageBitmap(page_id, wxArtProvider::GetBitmap(icon_name, wxART_OTHER, FromDIP(wxSize(16, 16))));
                m_notebook->Thaw();

                page_icons[page_id] = icon_name.ToStdString();
            }
            break;
        }
        case ID_CmdExecutorAddPage:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->AddPage(page_id, page_id + 1);

            cmd->SaveToTempAndReload();
            break;
        }        
        case ID_CmdExecutorDeletePage:
        {
            wxMessageDialog d(this, "Are you sure want to delete this page?", "Deleting", wxOK | wxCANCEL);
            int ret_code = d.ShowModal();
            if(ret_code == wxID_OK)  /* OK */
            {
                std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
                cmd->DeletePage(page_id);
                cmd->SaveToTempAndReload();
            }
            break;
        }
        case ID_CmdExecutorDuplicatePageBefore:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->CopyPage(page_id, page_id);

            cmd->SaveToTempAndReload();
            break;
        }
        case ID_CmdExecutorDuplicatePageAfter:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->CopyPage(page_id, page_id + 1);

            cmd->SaveToTempAndReload();
            break;
        }
    }
    return;
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
}

void CmdExecutorPanelBase::OnPostReload(uint8_t page, uint8_t cols, CommandPageNames& names, CommandPageIcons& icons)
{
    m_Pages[page - 1]->OnPostReloadUpdate();

    m_notebook->Freeze();
    m_notebook->SetPageText(page - 1, names.back());
    if(!icons.back().empty())
        m_notebook->SetPageBitmap(page - 1, wxArtProvider::GetBitmap(icons.back(), wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->Thaw();
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

void CmdExecutorPanelPage::OnPaint(wxPaintEvent& evt)
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
    menu.Append(ID_CmdExecutorAdd, "&Add", "Add command to selected column")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorAddSeparator, "&Add separator", "Add separator")->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorAddCol, "&Add col")->SetBitmap(wxArtProvider::GetBitmap(wxART_REMOVABLE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDeleteCol, "&Delete col")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorSave, "&Save")->SetBitmap(wxArtProvider::GetBitmap(wxART_FLOPPY, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorReload, "&Reload")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(14, 14))));

    const wxPoint pt = wxGetMousePosition();
    int mouseX = pt.x - this->GetScreenPosition().x;
    int mouseY = pt.y - this->GetScreenPosition().y;

    std::vector<wxSize> pos;
    uint8_t col = 1;
    uint8_t curr_sizer = 0xFF;
    for(auto& i : m_VertialBoxes)
    {
        wxPoint sizer_pos = i->GetPosition();
        wxSize sizer_size = i->GetSize();

        if(mouseX > sizer_pos.x && mouseX < (sizer_pos.x + sizer_size.x))
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
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->AddCommand(m_Id, col, 
                Command(std::format("New cmd {}", utils::random_mt(1, 1000)), "& ping 127.0.0.1 -n 3 > nul", "", utils::random_mt(0x0, 0xFFFFFF), 0xFFFFFF, false, "", 1.0f));

            OnPostReloadUpdate();
            break;
        }
        case ID_CmdExecutorAddSeparator:
        {           
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;

            wxTextEntryDialog d(this, "Specify separator width", "Add separator");
            //d.SetValidator(wxIntegerValidator<uint8_t>());
            d.SetValue("10");
            int ret = d.ShowModal();
            if(ret == wxID_OK)
            {
                try
                {
                    uint8_t separator_width = static_cast<uint8_t>(std::stoi(d.GetValue().ToStdString()));

                    cmd->AddSeparator(m_Id, col, Separator(separator_width));
                }
                catch(const std::exception& e)
                {
                    LOG(LogLevel::Error, "stoi exception: {}", e.what());
                }
            }

            OnPostReloadUpdate();
            break;
        }
        case ID_CmdExecutorAddCol:
        {           
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->AddCol(CmdExecutorPanelBase::m_CurrentPage, col);

            cmd->SaveToTempAndReload();
            break;
        }
        case ID_CmdExecutorDeleteCol:
        {           
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->DeleteCol(CmdExecutorPanelBase::m_CurrentPage, col - 1);

            cmd->SaveToTempAndReload();
            break;
        }
        case ID_CmdExecutorSave:
        {           
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->Save();
            LOG(LogLevel::Notification, "Commands has been saved");
            break;
        }        
        case ID_CmdExecutorReload:
        {           
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
            cmd->ReloadCommandsFromFile();
            LOG(LogLevel::Notification, "Commands has been reloaded");
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
    Execute(c);
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
    menu.Append(ID_CmdExecutorDuplicate, "&Duplicate")->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorMoveUp, "&Move Up")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorMoveDown, "&Move Down")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_CmdExecutorDelete, "&Delete")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    int ret = GetPopupMenuSelectionFromUser(menu);
    
    switch(ret)
    {
        case ID_CmdExecutorEdit:
        {
            edit_dlg->ShowDialog(c->GetName(), c->GetCmd(), c->GetColor(), c->GetBackgroundColor(), c->IsBold(), c->GetFontFace(), c->GetScale());
            if(!edit_dlg->IsApplyClicked()) return;

            const std::string cmd_name = edit_dlg->GetCmdName().ToStdString();
            const std::string cmd = edit_dlg->GetCmd().ToStdString();
            wxColor color = edit_dlg->GetTextColor();
            wxColor bg_color = edit_dlg->GetBgColor();
            bool is_bold = edit_dlg->IsBold();
            const std::string font_face = edit_dlg->GetFontFace().ToStdString();
            float scale = edit_dlg->GetScale();

            c->SetName(cmd_name).SetCmd(cmd).SetColor(WXCOLOR_TO_RGB(color.GetRGB())).SetBackgroundColor(WXCOLOR_TO_RGB(bg_color.GetRGB())).
                SetBold(is_bold).SetFontFace(font_face).SetScale(scale);

            UpdateCommandButon(c, btn, true);
            break;
        }
        case ID_CmdExecutorDuplicate:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;

            uint8_t col = 0xFF;
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
                                return true;
                            }
                        }
                        return false;
                    }, it->second);

                if(ret)
                {
                    col = it->first;
                    break;
                }
                else
                    ++it;
            }

            cmd->AddCommand(m_Id, col + 1, Command(*c));
            m_BaseGrid->Layout();
            break;
        }
        case ID_CmdExecutorMoveUp:
        {
            std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;

            uint8_t col = 0xFF;
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
                                return true;
                            }
                        }
                        return false;
                    }, it->second);

                if(ret)
                {
                    col = it->first;
                    break;
                }
                else
                    ++it;
            }

            cmd->AddCommand(m_Id, col + 1, Command(*c));
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

void CmdExecutorPanelPage::OnMiddleClick(wxMouseEvent& event)
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

    DBG("middleclick");
    Command* c = reinterpret_cast<Command*>(clientdata);
    if(c->GetParam().empty())
    {
        LOG(LogLevel::Error, "Param for command '{}' is empty!", c->GetName());
        return;
    }

    wxTextEntryDialog d(this, "New parameter", "Execute with new parameter");
    int ret = d.ShowModal();
    if(ret == wxID_OK)
    {
        c->SetParam(d.GetValue().ToStdString());
        Execute(c);
    }
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
        wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxString::Format("Col: %d", i));
        box_sizer->SetMinSize(wxSize(200, 200));
        m_BaseGrid->Add(box_sizer, wxSizerFlags(5).Expand());
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
    btn->Bind(wxEVT_MIDDLE_DOWN, &CmdExecutorPanelPage::OnMiddleClick, this);

    m_ButtonMap.emplace(col - 1, btn);
    m_VertialBoxes[col - 1]->Add(btn);
}

void CmdExecutorPanelPage::AddSeparatorElement(uint8_t col, Separator s)
{
    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, s.width), wxLI_HORIZONTAL);
    m_ButtonMap.emplace(col - 1, line);
    m_VertialBoxes[col - 1]->Add(line, wxSizerFlags(0).Expand());

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

    if(!c->GetFontFace().empty())
        font.SetFaceName(c->GetFontFace());
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
    btn->Disconnect(wxEVT_MIDDLE_DOWN);
    btn->Destroy();

    m_BaseGrid->Layout();
}

void CmdExecutorPanelPage::Execute(Command* c)
{
    ToggleAllButtonClickability(false);
    c->Execute();
    ToggleAllButtonClickability(true);
}

IconSelectionDialog::IconSelectionDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Select an icon")
{
    // Create the wxBitmapComboBox.

    wxArrayString empty_str;
    icon_combo_box = new wxBitmapComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, empty_str, 0);

    // Add all available wxART_* icons to the wxBitmapComboBox.
    for(const auto& art_name : art_names)
    {
        wxBitmap icon = wxArtProvider::GetBitmap(art_name, wxART_OTHER, FromDIP(wxSize(24, 24)));
        if(icon.IsOk()) {
            icon_combo_box->Append(art_name, icon);
        }
    }

    // Set the default icon.
    icon_combo_box->SetSelection(0);

    // Layout the wxBitmapComboBox.
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(icon_combo_box, wxSizerFlags().Expand());
    main_sizer->Add(CreateStdDialogButtonSizer(wxCANCEL | wxOK), wxSizerFlags().Right().Border()); /* wxOK */
    SetSizer(main_sizer);
}

void IconSelectionDialog::SelectIconByName(const wxString& name)
{
    int cnt = 0;
    for(const auto& art_name : art_names)
    {
        if(art_name == name)
        {
            wxBitmap icon = wxArtProvider::GetBitmap(art_name, wxART_OTHER, FromDIP(wxSize(24, 24)));
            if(icon.IsOk()) {
                icon_combo_box->SetSelection(cnt);
            }
        }
        cnt++;
    }
}

wxString IconSelectionDialog::GetSelectedIcon() 
{
    return icon_combo_box->GetStringSelection();
}