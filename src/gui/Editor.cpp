#include "pch.h"

wxBEGIN_EVENT_TABLE(GuiEditorMain, wxPanel)
EVT_MOTION(GuiEditorMain::OnMouseMotion)
//EVT_LEFT_DOWN(GuiEditorMain::OnMouseLeftDown)
EVT_PAINT(GuiEditorMain::OnPaint)
EVT_SIZE(GuiEditorMain::OnSize)
EVT_PG_CHANGED(ID_EditorPropGrid, GuiEditorMain::OnPropertyGridChange)
EVT_CONTEXT_MENU(GuiEditorMain::OnContextMenu)
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
	//m_mgr.SetManagedWindow(this);
	wxSize client_size = GetClientSize();
	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), Settings::Get()->window_size, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->Freeze();
	gui_editor = new GuiEditorMain(m_notebook);
	gui_cpp = new GuiEditorCpp(m_notebook);
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(EditorPanel::Changeing), NULL, this);
	m_notebook->AddPage(gui_editor, "Editor", false, wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(gui_cpp, "C++ Code", false, wxArtProvider::GetBitmap(wxART_PRINT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Thaw();
	
	GuiEditor::Get()->Init(gui_editor);
}

GuiEditorMain::GuiEditorMain(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	m_propertyGrid = new wxPropertyGrid(this, ID_EditorPropGrid, wxDefaultPosition, wxSize(350, 100), wxPG_SPLITTER_AUTO_CENTER | wxPG_STATIC_SPLITTER);
	wxPGProperty* pid = m_propertyGrid->Append(new wxPropertyCategory("Item"));
	pid->SetValue("Value");

	bSizer1->Add(m_propertyGrid, wxSizerFlags(10).Right().Border(wxRIGHT, 22));

	m_pgType = m_propertyGrid->Append(new wxStringProperty(wxT("Type"), wxPG_LABEL));
	m_propertyGrid->DisableProperty("Type");
	m_propertyGrid->MakeColumnEditable(2, true);

	int id = 0;
	wxString pos;
	wxString size;
	wxColour color;
	m_pgId = m_propertyGrid->Append(new wxIntProperty(wxT("ID"), wxPG_LABEL, id));
	m_pgName = m_propertyGrid->Append(new wxStringProperty(wxT("Name"), wxPG_LABEL));
	m_pgLabel = m_propertyGrid->Append(new wxStringProperty(wxT("Label"), wxPG_LABEL));
	m_pgPos = m_propertyGrid->Append(new wxStringProperty(wxT("Position"), wxPG_LABEL, pos));
	m_pgSize = m_propertyGrid->Append(new wxStringProperty(wxT("Size"), wxPG_LABEL, size));
	m_pgMinSize = m_propertyGrid->Append(new wxStringProperty(wxT("Min size"), wxPG_LABEL, size));
	m_pgMaxSize = m_propertyGrid->Append(new wxStringProperty(wxT("Max Size"), wxPG_LABEL, size));
	m_pgForegroundColor = m_propertyGrid->Append(new wxColourProperty(wxT("Fg. Colour"), wxPG_LABEL, color));
	m_pgBackgroundColor = m_propertyGrid->Append(new wxColourProperty(wxT("Bg. Colour"), wxPG_LABEL, color));
	m_pgFont = m_propertyGrid->Append(new wxFontProperty(wxT("Font"), wxPG_LABEL));
	m_pgTooltip = m_propertyGrid->Append(new wxStringProperty(wxT("Tooltip"), wxPG_LABEL));
	m_pgEnabled = m_propertyGrid->Append(new wxBoolProperty(wxT("Enabled"), wxPG_LABEL, true));
	m_pgHidden = m_propertyGrid->Append(new wxBoolProperty(wxT("Hidden"), wxPG_LABEL, false));

	Bind(wxEVT_CHAR_HOOK, &GuiEditorMain::OnKeyDown, this);

	this->SetSizerAndFit(bSizer1);
	this->Layout();
}

void GuiEditorMain::OnClick(wxMouseEvent& event)
{
	if(!m_propertyGrid->IsEditorFocused())
	{
		void* obj = event.GetEventObject();
		GuiEditor::Get()->OnClick(obj);
	}
	//event.Skip();
}

void GuiEditorMain::UpdatePropgrid(PropgripdParams_t&& params)
{
	wxWindow* w = params.w;

	std::string typeid_name(typeid(*w).name());
	size_t pos = typeid_name.find("wx", 0);  /* typeid(T).name() returns: "class wxButton * __ptr64" */
	size_t pointer = typeid_name.find("*", 0);
	std::string widget_type = typeid_name.substr(pos, pointer - pos - 1);
	m_pgType->SetValue(widget_type);
	m_pgId->SetValue(params.id);
	m_pgName->SetValue(*params.name);
	m_pgLabel->SetValue(dynamic_cast<wxControlBase*>(w)->GetLabelText());
	m_pgPos->SetValue(wxString::Format("%d,%d", w->GetPosition().x, w->GetPosition().y));
	m_pgSize->SetValue(wxString::Format("%d,%d", w->GetSize().x, w->GetSize().y));
	m_pgMinSize->SetValue(wxString::Format("%d,%d", w->GetMinSize().x, w->GetMinSize().y));
	m_pgMaxSize->SetValue(wxString::Format("%d,%d", w->GetMaxSize().x, w->GetMaxSize().y));

	wxVariant f_clr, b_clr, fnt;
	f_clr << w->GetForegroundColour();
	b_clr << w->GetBackgroundColour();
	fnt << w->GetFont();
	m_pgForegroundColor->SetValue(f_clr);
	m_pgBackgroundColor->SetValue(b_clr);
	m_pgFont->SetValue(fnt);
	m_pgTooltip->SetValue(w->GetToolTipText());
	m_pgEnabled->SetValue(w->IsEnabled());
	m_pgHidden->SetValue(!w->IsShown());
}

void GuiEditorMain::OnKeyDown(wxKeyEvent& event)
{
	if(!m_propertyGrid->IsEditorFocused())
	{
		int keycode = (int)event.GetKeyCode();
		GuiEditor::Get()->OnKeyDown(keycode);
	}
	event.Skip();
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

void GuiEditorMain::OnPropertyGridChange(wxPropertyGridEvent& event)
{
	wxPGProperty* property = event.GetProperty();
	GuiEditor::Get()->OnPropertyGridChange(property);
}

void GuiEditorMain::OnContextMenu(wxContextMenuEvent& evt)
{
	DBG("context menu\n");
}

void GuiEditorMain::OnSize(wxSizeEvent& event)
{
	event.Skip(true);
}

GuiEditorCpp::GuiEditorCpp(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
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
	this->SetSizerAndFit(bSizer1);
	this->Layout();
}

