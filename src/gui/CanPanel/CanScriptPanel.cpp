#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanScriptPanel, wxPanel)
EVT_FILEPICKER_CHANGED(ID_FilePickerCanScript, CanScriptPanel::OnFileSelected)
EVT_SIZE(CanScriptPanel::OnSize)
wxEND_EVENT_TABLE()

CanScriptPanel::CanScriptPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    m_Script = std::make_unique<CanScriptHandler>(*this);

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
            catch(const std::exception& e)
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
            catch(const std::exception& e)
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

CanScriptPanel::~CanScriptPanel()
{
    m_Output = nullptr;
}

void CanScriptPanel::AddToLog(std::string str)
{
    if(m_Output)
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