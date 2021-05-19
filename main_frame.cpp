#include <boost/asio.hpp>

#include "Server.h"

#include "main_frame.h"
#include <wx/valnum.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/richtooltip.h>
#include <wx/clipbrd.h>
#include <wx/richmsgdlg.h>
#include <wx/aui/aui.h>
#include <wx/filepicker.h>
#include <wx/tglbtn.h>
#include <wx/socket.h>
#include <any>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/aui/aui.h>
#include "wx/treelist.h"
#include "wx/treectrl.h"
#include <wx/dirctrl.h>
#include <wx/xml/xml.h>
#include<boost/algorithm/string.hpp>

#include "Notification.h"
#include "StructParser.h"

#include <boost/algorithm/string.hpp>
#include <assert.h>

constexpr int WINDOW_SIZE_X = 800;
constexpr int WINDOW_SIZE_Y = 600;

enum
{
	ID_Hello = 1,
	ID_Quit,
	ID_UpdateMousePosText,
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Hello, MyFrame::OnHelp)
EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MainPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(GraphPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(EscaperPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ParserPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(LogPanel, wxPanel)
wxEND_EVENT_TABLE()

#define WX_SIZERPADDING(sizer) \
	sizer->Add(new wxStaticText(this, wxID_ANY, ""), 0, wxALL, 5);

void MyFrame::OnHelp(wxCommandEvent& event)
{
	wxMessageBox("This is a simple program which made for improving my coding experience.\n\
		Feel free to re(use) it in any way what you want.", "Help");
}

void MyFrame::OnClose(wxCloseEvent& event)
{
	ExitProcess(0);
}

void MyFrame::OnTimer(wxTimerEvent& event)
{
	int sel = ctrl->GetSelection();
	HWND foreground = GetForegroundWindow();
	if(sel == 3 && foreground)
	{
		POINT p;
		if(::GetCursorPos(&p))
		{
			if(::ScreenToClient(foreground, &p))
			{
				RECT rect;
				if(GetWindowRect(foreground, &rect))
				{
					int width = rect.right - rect.left;
					int height = rect.bottom - rect.top;
					wxString str = wxString::Format(wxT("Mouse: %d,%d\n\nRect: %d,%d"), p.x, p.y, width, height);
					macro_panel->m_MousePos->SetLabelText(str);
					if(GetAsyncKeyState(VK_SCROLL))
					{
						LOGMSG(normal, str.ToStdString().c_str());
					}
				}
			}
		}
	}
}

void MyFrame::SetIconTooltip(const wxString &str)
{
	if(!notification->SetIcon(wxIcon(wxT("aaaa")), str))
	{
		wxLogError("Could not set icon.");
	}
}

MyFrame::MyFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition,
		wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y))
{
	notification = new NotificationIcon();
	notification->SetMainFrame(this);
	SetIconTooltip(wxT("No measurements"));
	
	m_mgr.SetManagedWindow(this);

	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Hello, "&Read help\tCtrl-H", "Read description about this program");
	menuFile->Append(wxID_EXIT);
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	SetMenuBar(menuBar);
	
	wxMenu* menu = new wxMenu;

	menu->AppendSeparator();
	menu->Append(ID_Quit, _("E&xit"));

	wxMenuBar* menuBar_ = new wxMenuBar;
	menuBar_->Append(menu, _("&File"));

	SetMenuBar(menuBar);
	CreateStatusBar();
	SetStatusText("Welcome in CustomKeyboard");

	main_panel = new MainPanel(this);
	graph_panel = new GraphPanel(this);
	escape_panel = new EscaperPanel(this);
	macro_panel = new MacroPanel(this);
	parser_panel = new ParserPanel(this);
	log_panel = new LogPanel(this);

	wxSize client_size = GetClientSize();
	ctrl = new wxAuiNotebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), FromDIP(wxSize(430, 200)), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	ctrl->Freeze();
	ctrl->AddPage(main_panel, "Main page", false);
	ctrl->AddPage(graph_panel, "Graph page", false);
	ctrl->AddPage(escape_panel, "C StrEscape", false);
	ctrl->AddPage(macro_panel, "Custom Macro", false);
	ctrl->AddPage(parser_panel, "Sturct Parser", false);
	ctrl->AddPage(log_panel, "Log", false);
	ctrl->Thaw();
	ctrl->SetSelection(4);

	m_timer = new wxTimer(this, ID_UpdateMousePosText);
	Connect(m_timer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::OnTimer), NULL, this);
	m_timer->Start(100, false);
}

MainPanel::MainPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_RefreshButton = new wxButton(this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
	m_RefreshButton->SetToolTip("Request new measurements");
	bSizer1->Add(m_RefreshButton, 0, wxALL, 5);
	
	m_RefreshButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Server::Get()->BroadcastMessage("SEND_SENSOR_DATA");
		});

#define ADD_MEASUREMENT_TEXT(var_name, name) \
	var_name = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(-1, -1), 0); \
	var_name->Wrap(-1); \
	var_name->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
	var_name->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND)); \
	bSizer1->Add(var_name, 0, wxALL, 5)

	ADD_MEASUREMENT_TEXT(m_textTemp, "Temperature: N/A");
	ADD_MEASUREMENT_TEXT(m_textHum, "Humidity: N/A");
	ADD_MEASUREMENT_TEXT(m_textCO2, "CO2: N/A");
	ADD_MEASUREMENT_TEXT(m_textVOC, "VOC: N/A");
	ADD_MEASUREMENT_TEXT(m_textPM25, "PM25: N/A");
	ADD_MEASUREMENT_TEXT(m_textPM10, "PM10: N/A");
	ADD_MEASUREMENT_TEXT(m_textLux, "Lux: N/A");
	ADD_MEASUREMENT_TEXT(m_textCCT, "CCT: N/A");

	this->SetSizer(bSizer1);
}

GraphPanel::GraphPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{	
#if 0
	// might be useful for something in the future
#endif
}

EscaperPanel::EscaperPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_IsEscapePercent = new wxCheckBox(this, wxID_ANY, wxT("Escape %?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsEscapePercent, 0, wxALL, 5);
	m_IsBackslashAtEnd = new wxCheckBox(this, wxID_ANY, wxT("Add \\ at the end of line?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsBackslashAtEnd, 0, wxALL, 5);

	m_StyledTextCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(640, 320), 0, wxEmptyString);
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

	bSizer1->Add(m_StyledTextCtrl);

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
			if (m_IsBackslashAtEnd->IsChecked())
			{
				
				boost::algorithm::replace_all(str, "\x0D\x0A", "\x5C\x0D\x0A");
			}
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(str));
				wxTheClipboard->Close();
			}
		});
}

MacroPanel::MacroPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	m_MousePos = new wxStaticText(this, wxID_ANY, "Pos: 0,0");
	bSizer1->Add(m_MousePos);

	this->SetSizer(bSizer1);
	this->Layout();
	/*
	this->Bind(wxEVT_CHAR_HOOK, &MyPanel::OnKeyDown, this);
	*/
}

ParserPanel::ParserPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_IsModbus = new wxCheckBox(this, wxID_ANY, wxT("Is modbus?"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_IsModbus, 0, wxALL, 5);
	WX_SIZERPADDING(fgSizer1);

	const std::vector<wxString> vec_pointer_sizes = { "Ptr size: 1", "Ptr size: 2", "Ptr size: 4", "Ptr size: 8" };
	m_PointerSize = new wxComboBox(this, wxID_ANY, wxT("Ptr size: 4"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	m_PointerSize->Set(vec_pointer_sizes);
	m_PointerSize->SetSelection(2);
	m_PointerSize->SetHelpText("Select pointer size");
	fgSizer1->Add(m_PointerSize, 0, wxALL, 5);
	WX_SIZERPADDING(fgSizer1);

	m_StructurePadding = new wxSpinCtrl(this, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000, 1);
	m_StructurePadding->SetHelpText("Select structure padding");
	fgSizer1->Add(m_StructurePadding, 0, wxALL, 5);
	WX_SIZERPADDING(fgSizer1);

	m_StyledTextCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(320, 320), 0, wxEmptyString);

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
	fgSizer1->Add(m_StyledTextCtrl);

	m_Output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(320, 320), wxTE_MULTILINE | wxTE_READONLY);
	fgSizer1->Add(m_Output, 0, wxALL, 5);

	m_OkButton = new wxButton(this, wxID_ANY, wxT("Generate"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_OkButton, 0, wxALL, 5);

	this->SetSizer(fgSizer1);
	this->Layout();
	this->Centre(wxBOTH);

	m_OkButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			const uint8_t pointer_sizes[4] = { 1, 2, 4, 8 };
			bool is_modbus = m_IsModbus->GetValue();

			assert(m_PointerSize->GetSelection() < 4);

			int pointer_size = pointer_sizes[m_PointerSize->GetSelection()];
			int struct_padding = m_StructurePadding->GetValue();

			wxString str = m_StyledTextCtrl->GetText();
			std::string input(str.mb_str());
			std::string output;
			try
			{
				StructParser::Get()->ParseStructure(input, output);
			}
			catch(std::exception& e)
			{
				LOGMSG(critical, "Exception {}", e.what());
			}

			wxString wxout(output);
			m_Output->Clear();
			m_Output->SetLabelText(wxout);
		});
}

LogPanel::LogPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	m_Log = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(600, 400), 0, 0, wxLB_EXTENDED | wxLB_HSCROLL | wxLB_NEEDED_SB);
	m_Log->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
		{
			wxClipboard* clip = new wxClipboard();
			clip->Clear();
			clip->SetData(new wxTextDataObject(m_Log->GetString(m_Log->GetSelection())));
			clip->Flush();
			clip->Close();
		});
	bSizer1->Add(m_Log, 0, wxALL, 5);
	this->SetSizer(bSizer1);
	this->Layout();

	m_ClearButton = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	m_ClearButton->SetToolTip("Clear log box");
	bSizer1->Add(m_ClearButton, 0, wxALL, 5);

	m_ClearButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			m_Log->Clear();
		});

}