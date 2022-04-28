#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ParserPanel, wxPanel)
EVT_FILEPICKER_CHANGED(ID_FilePicker, ParserPanel::OnFileSelected)
EVT_SIZE(ParserPanel::OnSize)
wxEND_EVENT_TABLE()

ParserPanel::ParserPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_IsModbus = new wxCheckBox(this, wxID_ANY, wxT("Is modbus?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsModbus, 0, wxALL, 5);

	const std::vector<wxString> vec_pointer_sizes = { "Ptr size: 1", "Ptr size: 2", "Ptr size: 4", "Ptr size: 8" };
	m_PointerSize = new wxComboBox(this, wxID_ANY, wxT("Ptr size: 4"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	m_PointerSize->Set(vec_pointer_sizes);
	m_PointerSize->SetSelection(2);
	m_PointerSize->SetHelpText("Select pointer size");
	bSizer1->Add(m_PointerSize, 0, wxALL, 5);

	m_StructurePadding = new wxSpinCtrl(this, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000, 1);
	m_StructurePadding->SetHelpText("Select structure padding");
	bSizer1->Add(m_StructurePadding, 0, wxALL, 5);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Select a file, paste it's content or Drag'n'Drop to textbox below\nWhen done, click on Generate!"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
	m_FilePicker = new wxFilePickerCtrl(this, ID_FilePicker, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE);
	bSizer1->Add(m_FilePicker, 0, wxALL, 5);

	m_StyledTextCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(310, 310), 0, wxEmptyString);
	m_StyledTextCtrl->SetUseTabs(true);
	m_StyledTextCtrl->SetTabWidth(4);
	m_StyledTextCtrl->SetIndent(4);
	m_StyledTextCtrl->SetTabIndents(true);
	m_StyledTextCtrl->SetBackSpaceUnIndents(true);
	m_StyledTextCtrl->SetViewEOL(false);
	m_StyledTextCtrl->SetViewWhiteSpace(false);
	m_StyledTextCtrl->SetMarginWidth(2, 0);
	m_StyledTextCtrl->SetIndentationGuides(true);
	m_StyledTextCtrl->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
	m_StyledTextCtrl->SetMarginMask(1, wxSTC_MASK_FOLDERS);
	m_StyledTextCtrl->SetMarginWidth(1, 16);
	m_StyledTextCtrl->SetMarginSensitive(1, true);
	m_StyledTextCtrl->SetProperty(wxT("fold"), wxT("1"));
	m_StyledTextCtrl->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
	m_StyledTextCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	m_StyledTextCtrl->SetMarginWidth(0, m_StyledTextCtrl->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
	m_StyledTextCtrl->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("BLACK")));
	m_StyledTextCtrl->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
	m_StyledTextCtrl->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("BLACK")));
	m_StyledTextCtrl->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
	m_StyledTextCtrl->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("BLACK")));
	m_StyledTextCtrl->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
	m_StyledTextCtrl->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("BLACK")));
	m_StyledTextCtrl->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
	m_StyledTextCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
	m_StyledTextCtrl->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	m_StyledTextCtrl->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	// Set the lexer to the C++ lexer
	m_StyledTextCtrl->SetLexer(wxSTC_LEX_CPP);

	// Set the color to use for various elements
	m_StyledTextCtrl->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColor(60, 162, 2));
	m_StyledTextCtrl->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColor(0, 0, 255));
	m_StyledTextCtrl->StyleSetForeground(wxSTC_C_STRING, wxColor(255, 60, 10));
	m_StyledTextCtrl->StyleSetForeground(wxSTC_C_WORD, wxColor(0, 0, 255));
	m_StyledTextCtrl->StyleSetForeground(wxSTC_C_USERLITERAL, wxColor(0, 255, 255));

	// Give a list of keywords. They will be given the style specified for
	// wxSTC_C_WORD items.
	m_StyledTextCtrl->SetKeyWords(0, wxT("return int float double char this new delete goto for while do if else uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t"));
#ifdef _WIN32
	m_StyledTextCtrl->DragAcceptFiles(true);  /* This one doesn't work with GTK for some reason... */
#endif
	m_StyledTextCtrl->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(ParserPanel::OnFileDrop), NULL, this);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	bSizer2->Add(m_StyledTextCtrl, wxSizerFlags(1).Top().Expand());

	m_Output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(310, 310), wxTE_MULTILINE | wxTE_READONLY);
	bSizer2->Add(m_Output, wxSizerFlags(1).Expand());
	bSizer1->Add(bSizer2, wxSizerFlags(1).Expand());

	m_OkButton = new wxButton(this, wxID_ANY, wxT("Generate"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_OkButton);

	this->SetSizerAndFit(bSizer1);
	this->Layout();

	m_OkButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			const uint8_t pointer_sizes[4] = { 1, 2, 4, 8 };
			bool is_modbus = m_IsModbus->GetValue();

			assert(m_PointerSize->GetSelection() < 4);

			size_t pointer_size = pointer_sizes[m_PointerSize->GetSelection()];
			int struct_padding = m_StructurePadding->GetValue();

			wxString str = m_StyledTextCtrl->GetText();

			std::string input;
			if(!path.empty())
			{
				std::ifstream f(path.ToStdString(), std::ios::in | std::ios::binary);
				if(f)
					input = { (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>() };
			}
			else
			{
				input = str.mb_str();
			}

			std::string output;
			try
			{
				StructParser::Get()->ParseStructure(input, output, struct_padding, pointer_size);
			}
			catch(std::exception& e)
			{
				LOGMSG(critical, "Exception {}", e.what());
				wxMessageDialog(this, fmt::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}

			wxString wxout(output);
			m_Output->Clear();
			m_Output->SetLabelText(wxout);
			m_OkButton->SetForegroundColour(*wxBLACK);
			path.Clear();
		});
}

void ParserPanel::OnFileDrop(wxDropFilesEvent& event)
{
	if(event.GetNumberOfFiles() > 0)
	{
		wxString* dropped = event.GetFiles();
		path = *dropped;
		m_OkButton->SetForegroundColour(*wxRED);
	}
}

void ParserPanel::OnFileSelected(wxFileDirPickerEvent& event)
{
	path = event.GetPath();
	m_OkButton->SetForegroundColour(*wxRED);
}

void ParserPanel::OnSize(wxSizeEvent& event)
{
	wxSize a = event.GetSize();
	event.Skip(true);
}