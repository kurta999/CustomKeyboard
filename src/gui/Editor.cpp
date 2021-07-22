#include "pch.h"

wxBEGIN_EVENT_TABLE(GuiEditorMain, wxPanel)
EVT_MOTION(GuiEditorMain::OnMouseMotion)
EVT_LEFT_DOWN(GuiEditorMain::OnMouseLeftDown)
EVT_PAINT(GuiEditorMain::OnPaint)
wxEND_EVENT_TABLE()

void EditorPanel::Changeing(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if(sel == 1)
	{
		wxString str;	
		GuiEditor::Get()->GenerateCode(str);
		gui_cpp->m_StyledTextCtrl->ClearAll();
		gui_cpp->m_StyledTextCtrl->AddText(str);
	}
}

EditorPanel::EditorPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	wxAuiNotebook* m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	gui_editor = new GuiEditorMain(m_notebook);
	gui_cpp = new GuiEditorCpp(m_notebook);
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(EditorPanel::Changeing), NULL, this);
	m_notebook->AddPage(gui_editor, "Editor", false, wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(gui_cpp, "C++ Code", false, wxArtProvider::GetBitmap(wxART_PRINT, wxART_OTHER, FromDIP(wxSize(16, 16))));

	GuiEditor::Get()->Init(gui_editor);
}

GuiEditorMain::GuiEditorMain(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	Bind(wxEVT_CHAR_HOOK, &GuiEditorMain::OnKeyDown, this);
}

void GuiEditorMain::OnClick(wxMouseEvent& event)
{
	void* obj = event.GetEventObject();
	GuiEditor::Get()->OnClick(obj);
}

void GuiEditorMain::OnKeyDown(wxKeyEvent& event)
{
	int keycode = (int)event.GetKeyCode();
	GuiEditor::Get()->OnKeyDown(keycode);
}

void GuiEditorMain::OnMouseMotion(wxMouseEvent& event)
{
	wxPoint pos = ScreenToClient(::wxGetMousePosition());
	wxMouseState mouse = wxGetMouseState();
	if(mouse.LeftIsDown())
		GuiEditor::Get()->OnMouseMotion(pos);
}

void GuiEditorMain::OnMouseLeftDown(wxMouseEvent& event)
{

}

void GuiEditorMain::OnPaint(wxPaintEvent& event)
{
	GuiEditor::Get()->OnPaint();
}

GuiEditorCpp::GuiEditorCpp(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	m_StyledTextCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(1024, 900), 0, wxEmptyString);
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

}

