#include "pch.h"

EscaperPanel::EscaperPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_IsEscapePercent = new wxCheckBox(this, wxID_ANY, wxT("Escape %?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsEscapePercent, 0, wxALL, 5);
	m_IsBackslashAtEnd = new wxCheckBox(this, wxID_ANY, wxT("Add \\ at the end of line?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsBackslashAtEnd, 0, wxALL, 5);

	m_StyledTextCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
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

	// Give a list of keywords. They will be given the style specified for
	// wxSTC_C_WORD items.
	m_StyledTextCtrl->SetKeyWords(0, wxT("return int char this new"));

	bSizer1->Add(m_StyledTextCtrl, wxSizerFlags(1).Left().Expand());

	m_OkButton = new wxButton(this, wxID_ANY, wxT("Escape"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_OkButton, 0, wxALL, 5);

	this->SetSizer(bSizer1);
	this->Layout();
	m_OkButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			std::string str = m_StyledTextCtrl->GetText().ToStdString();
			boost::algorithm::replace_all(str, "\\", "\\\\");
			boost::algorithm::replace_all(str, "\"", "\x5C\""); /* \x5C = \ ASCII code, I was not able to type write this here */

			if(m_IsEscapePercent->IsChecked())
				boost::algorithm::replace_all(str, "%", "%%");
			if(m_IsBackslashAtEnd->IsChecked())
			{
				boost::algorithm::replace_all(str, "\x0D\x0A", "\x5C\x0D\x0A");
			}
			if(wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(str));
				MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
				{
					std::lock_guard lock(frame->mtx);
					frame->pending_msgs.push_back({ (uint8_t)StringEscaped });
				}
				wxTheClipboard->Close();
			}
		});
}
