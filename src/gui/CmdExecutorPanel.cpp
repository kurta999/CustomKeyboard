#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CmdExecutorPanel, wxPanel)
EVT_SIZE(CmdExecutorPanel::OnSize)
wxEND_EVENT_TABLE()

constexpr size_t MAX_CMD_LEN_FOR_BUTTON = 16;

CmdExecutorPanel::CmdExecutorPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{
    CmdExecutor* cmd = wxGetApp().cmd_executor;
    cmd->SetMediator(this);
    ReloadCommands();
    Show();
}

void CmdExecutorPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void CmdExecutorPanel::ToggleAllButtonClickability(bool toggle)
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

void CmdExecutorPanel::OnClick(wxCommandEvent& event)
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

    std::string* retval = reinterpret_cast<std::string*>(clientdata);
    ToggleAllButtonClickability(false);
#ifdef _WIN32 
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;  // Requires STARTF_USESHOWWINDOW in dwFlags.
    si.wShowWindow = SW_SHOW;  // Prevents cmd window from flashing.

    PROCESS_INFORMATION pi = { 0 };
    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)std::format("C:\\windows\\system32\\cmd.exe /c {}", *retval).c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
    if(fSuccess)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        LOG(LogLevel::Error, "CreateProcess failed with error code: {}", GetLastError());
    }
#else
    utils::exec(retval->c_str());
#endif
    ToggleAllButtonClickability(true);
}

void CmdExecutorPanel::ReloadCommands()
{
    CmdExecutor* cmd = wxGetApp().cmd_executor;
    cmd->ReloadCommandsFromFile();
}

void CmdExecutorPanel::OnPreReload(uint8_t cols)
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

void CmdExecutorPanel::OnCommandLoaded(uint8_t col, CommandTypes cmd)
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

void CmdExecutorPanel::OnPostReload(uint8_t cols)
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

void CmdExecutorPanel::AddCommandElement(uint8_t col, Command* c)
{
    wxButton* btn = new wxButton(this, wxID_ANY, !c->name.empty() ? c->name : c->cmd.substr(0, MAX_CMD_LEN_FOR_BUTTON), wxDefaultPosition, wxDefaultSize);
    btn->SetForegroundColour(wxColour(boost::endian::endian_reverse(c->color << 8)));  /* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
    btn->SetBackgroundColour(wxColour(boost::endian::endian_reverse(c->bg_color << 8)));

    wxFont font = btn->GetFont();
    bool reapply_font = false;
    if(c->is_bold)
    {
        font.SetWeight(wxFONTWEIGHT_BOLD);
        reapply_font = true;
    }
    if(c->scale != 1.0f)
    {
        font.Scale(c->scale);
        reapply_font = true;
    }

    if(reapply_font)
        btn->SetFont(font);

    btn->SetClientData((void*)&c->cmd);
    btn->Bind(wxEVT_BUTTON, &CmdExecutorPanel::OnClick, this);

    m_ButtonMap.emplace(col - 1, btn);
    m_VertialBoxes[col - 1]->Add(btn);
}

void CmdExecutorPanel::AddSeparatorElement(uint8_t col, Separator s)
{
    wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, s.width), wxLI_HORIZONTAL);
    m_ButtonMap.emplace(col - 1, line);
    m_VertialBoxes[col - 1]->Add(line);
}