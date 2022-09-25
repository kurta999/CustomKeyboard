#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CmdExecutorPanel, wxPanel)
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

void CmdExecutorPanel::ToggleAllButtonClickability(bool toggle)
{
    for(auto& i : m_ButtonMap)
    {
        if(std::holds_alternative<wxButton*>(i.second))
        {
            wxButton* btn = std::get<wxButton*>(i.second);
            if(btn)
                btn->Enable(toggle);
        }
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
    
    DBG("click message: %s\n", retval->c_str());
    //utils::exec(retval->c_str());

    ToggleAllButtonClickability(false);
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW; // Prevents cmd window from flashing.
    // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };
    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)std::format("C:\\windows\\system32\\cmd.exe /c {}", *retval).c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
        //delete m_BaseGrid;
        //m_BaseGrid = nullptr;

        for(auto& i : m_ButtonMap)
        {
            if(std::holds_alternative<wxButton*>(i.second))
            {
                wxButton* btn = std::get<wxButton*>(i.second);
                bool ret = btn->Destroy();
            } 
            else if(std::holds_alternative<wxStaticLine*>(i.second))
            {
                wxStaticLine* btn = std::get<wxStaticLine*>(i.second);
                bool ret = btn->Destroy();
            }
        }

        m_VertialBoxes.clear();
        m_ButtonMap.clear();
    }

    m_BaseGrid = new wxGridSizer(cols);
    SetSizer(m_BaseGrid);
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
    if(std::holds_alternative<std::shared_ptr<Command>>(cmd))
    {
        std::shared_ptr<Command> c = std::get<std::shared_ptr<Command>>(cmd);
        
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
    else if(std::holds_alternative<Separator>(cmd))
    {
        Separator s = std::get<Separator>(cmd);
        wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(210, s.width), wxLI_HORIZONTAL);
        
        m_ButtonMap.emplace(col - 1, line);
        m_VertialBoxes[col - 1]->Add(line);
    }
}

void CmdExecutorPanel::OnPostReload(uint8_t cols)
{
    for(auto& i : m_VertialBoxes)
    {
        i->Layout();
    }
    m_BaseGrid->Layout();
}