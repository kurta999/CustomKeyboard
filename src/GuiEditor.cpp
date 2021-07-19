#include "pch.h"


const wxString choices[] = { "wxButton", "wxComboBox", "wxChoise", "wxListBox", "wxCheckBox", "wxRadioButton", "wxStaticLine", "wxSlider", "wxGauge", "wxText",
"wxSpinControl" , "wxSpinCtrlDouble" , "wxTextControl" , "wxToggleButton" , "wxSearchCtrl" , "wxColorPicker" , "wxFontPicker" , "wxFilePicker" , "wxDirPicker" , "wxDatePicker",
"wxTimePicker" , "wxCalendarCtrl" , "wxGenericDirCtrl", "wxSpinButton" };

static const wxString button_style_flags[] = {
	"wxBORDER_NONE",
	"wxBU_BOTTOM",
	"wxBU_EXACTFIT",
	"wxBU_LEFT",
	"wxBU_NOTEXT",
	"wxBU_RIGHT",
	"wxBU_TOP",
};
static const long button_style_values[] = {
	wxBORDER_NONE,
	wxBU_BOTTOM,
	wxBU_EXACTFIT,
	wxBU_LEFT,
	wxBU_NOTEXT,
	wxBU_RIGHT,
	wxBU_TOP,
};

static const wxString slider_style_flags[] = {
	"wxSL_AUTOTICKS",
	"wxSL_BOTH",
	"wxSL_BOTTOM",
	"wxSL_HORIZONTAL",
	"wxSL_INVERSE",
	"wxSL_LABELS",
	"wxSL_LEFT",
	"wxSL_MIN_MAX_LABELS",
	"wxSL_RIGHT",
	"wxSL_SELRANGE",
	"wxSL_TOP",
	"wxSL_VALUE_LABEL",
	"wxSL_VERTICAL",
};
static const long slider_style_values[] = {
	wxSL_AUTOTICKS,
	wxSL_BOTH,
	wxSL_BOTTOM,
	wxSL_HORIZONTAL,
	wxSL_INVERSE,
	wxSL_LABELS,
	wxSL_LEFT,
	wxSL_MIN_MAX_LABELS,
	wxSL_RIGHT,
	wxSL_SELRANGE,
	wxSL_TOP,
	wxSL_VALUE_LABEL,
	wxSL_VERTICAL,
};

static const wxString statictext_style_flags[] = {
	"wxALIGN_CENTER_HORIZONTAL",
	"wxALIGN_LEFT",
	"wxALIGN_RIGHT",
	"wxST_ELLIPSIZE_END",
	"wxST_ELLIPSIZE_MIDDLE",
	"wxST_ELLIPSIZE_START",
	"wxST_NO_AUTORESIZE",
};
static const long statictext_style_values[] = {
	wxALIGN_CENTER_HORIZONTAL,
	wxALIGN_LEFT,
	wxALIGN_RIGHT,
	wxST_ELLIPSIZE_END,
	wxST_ELLIPSIZE_MIDDLE,
	wxST_ELLIPSIZE_START,
	wxST_NO_AUTORESIZE,
};

static const wxString textctrl_style_flags[] = {
	"wxTE_AUTO_URL",
	"wxTE_BESTWRAP",
	"wxTE_CAPITALIZE",
	"wxTE_CENTER",
	"wxTE_CHARWRAP",
	"wxTE_DONTWRAP",
	"wxTE_LEFT",
	"wxTE_MULTILINE",
	"wxTE_NOHIDESEL",
	"wxTE_NO_VSCROLL",
	"wxTE_PASSWORD",
	"wxTE_PROCESS_ENTER",
	"wxTE_PROCESS_TAB",
	"wxTE_READONLY",
	"wxTE_RICH",
	"wxTE_RICH2",
	"wxTE_RIGHT",
	"wxTE_WORDWRAP",
};
static const long textctrl_style_values[] = {
	wxHSCROLL,
	wxTE_AUTO_URL,
	wxTE_BESTWRAP,
	wxTE_CAPITALIZE,
	wxTE_CENTER,
	wxTE_CHARWRAP,
	wxTE_DONTWRAP,
	wxTE_LEFT,
	wxTE_MULTILINE,
	wxTE_NOHIDESEL,
	wxTE_NO_VSCROLL,
	wxTE_PASSWORD,
	wxTE_PROCESS_ENTER,
	wxTE_PROCESS_TAB,
	wxTE_READONLY,
	wxTE_RICH,
	wxTE_RICH2,
	wxTE_RIGHT,
	wxTE_WORDWRAP,
};

static const wxString combobox_style_flags[] = {
	"wxCB_DROPDOWN",
	"wxCB_READONLY",
	"wxCB_SIMPLE",
	"wxCB_SORT",
	"wxTE_PROCESS_ENTER",
};
static const long combobox_style_values[] = {
	wxCB_DROPDOWN,
	wxCB_READONLY,
	wxCB_SIMPLE,
	wxCB_SORT,
	wxTE_PROCESS_ENTER,
};

static const wxString choice_style_flags[] = {
	"wxCB_SORT",
};
static const long choice_style_values[] = {
	wxCB_SORT
};

static const wxString checkbox_style_flags[] = {
	"wxALIGN_RIGHT",
	"wxCHK_2STATE",
	"wxCHK_3STATE",
	"wxCHK_ALLOW_3RD_STATE_FOR_USER",
};
static const long checkbox_style_values[] = {
	wxALIGN_RIGHT,
	wxCHK_2STATE,
	wxCHK_3STATE,
	wxCHK_ALLOW_3RD_STATE_FOR_USER,
};

static const wxString gauge_style_flags[] = {
	"wxGA_HORIZONTAL",
	"wxGA_SMOOTH",
	"wxGA_VERTICAL",
};
static const long gauge_style_values[] = {
	wxGA_HORIZONTAL,
	wxGA_SMOOTH,
	wxGA_VERTICAL,
};

std::map<size_t, uint16_t> Widget::item_ids;

void GuiEditor::Init(GuiEditorMain* panel_)
{
	panel = panel_;
}

void GuiEditor::OnClick(void* object)
{
	Widget* t = FindwxText((wxObject*)object);
	if(t != nullptr)
	{
		m_SelectedWidget = (wxObject*)object;
		MarkSelectedItem();
	}
}

void GuiEditor::OnMouseMotion(wxPoint& pos)
{
	Widget* t = FindwxText();
	if(t != nullptr)
	{
		((wxStaticText*)m_SelectedWidget)->SetPosition(pos); /* since there is no reflection, use what works... */
		MarkSelectedItem();
	}
}

void GuiEditor::OnKeyDown(int keycode)
{
	Widget* t = FindwxText();
	switch(keycode)
	{
		case 388: /* + */
		{
			LOGMSG(notification, "Move speed has increased to {}.\n", ++m_speed);
			break;
		}
		case 390: /* - */
		{
			if(!--m_speed)
				m_speed = 1;
			LOGMSG(notification, "Move speed has decresed to {}.\n", m_speed);
			break;
		}
		case 65: /* A */
		{
			wxSingleChoiceDialog s(panel, "Add widget", "Add widget 2222", WXSIZEOF(choices), choices);
			if(s.ShowModal() == wxID_OK)
			{
				int sel = s.GetSelection();
				LOGMSG(notification, "Selected {}", choices[sel]);

				switch(sel)
				{
					case 0:
						GuiEditor::Get()->AddWidget<wxButton>(panel, wxID_ANY, wxT("btn"), wxPoint(0, 20), wxSize(25, 25), 0);
						break;
					case 1:
						GuiEditor::Get()->AddWidget<wxComboBox>(panel, wxID_ANY, wxT("combo"), wxPoint(0, 20), wxSize(25, 25), 0);
						break;
					case 2:
					{
						wxArrayString m_choiceChoices2;
						m_choiceChoices2.Add("First");
						m_choiceChoices2.Add("Second");
						GuiEditor::Get()->AddWidget<wxChoice>(panel, wxID_ANY, wxPoint(80, 815), wxSize(40, 25), m_choiceChoices2, 0);
						break;
					}
					case 3:
						GuiEditor::Get()->AddWidget<wxListBox>(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
						break;
					case 4:
						GuiEditor::Get()->AddWidget<wxCheckBox>(panel, wxID_ANY, wxT("Check Me!"), wxDefaultPosition, wxDefaultSize, 0);
						break;
					case 5:
						GuiEditor::Get()->AddWidget<wxRadioButton>(panel, wxID_ANY, wxT("RadioBtn"), wxDefaultPosition, wxDefaultSize, 0);
						break;
					case 6:
						GuiEditor::Get()->AddWidget<wxStaticLine>(panel, wxID_ANY, wxPoint(180, 815), wxSize(25, 25), 0);
						break;
					case 7:
						GuiEditor::Get()->AddWidget<wxSlider>(panel, wxID_ANY, 50, 0, 100, wxPoint(210, 815), wxSize(25, 25), 0);
						break;
					case 8:
						GuiEditor::Get()->AddWidget<wxGauge>(panel, wxID_ANY, 100, wxPoint(240, 815), wxSize(25, 25), 0);
						break;
					case 9:
						GuiEditor::Get()->AddWidget<wxStaticText>(panel, wxID_ANY, wxT("wxStaticText"), wxPoint(0, 785), wxDefaultSize, 0);
						break;
					case 10:
						GuiEditor::Get()->AddWidget<wxSpinCtrl>(panel, wxID_ANY, wxEmptyString, wxPoint(76, 785), wxSize(25, 25), wxSP_ARROW_KEYS, 1, 255, 1);
						break;
					case 11:
						GuiEditor::Get()->AddWidget<wxSpinCtrlDouble>(panel, wxID_ANY, wxEmptyString, wxPoint(0, 70), wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);
						break;
					case 12:
						GuiEditor::Get()->AddWidget<wxTextCtrl>(panel, wxID_ANY, wxT("wxTextCtrl"), wxPoint(140, 785), wxSize(25, 25), 0);
						break;
					case 13:
						GuiEditor::Get()->AddWidget<wxToggleButton>(panel, wxID_ANY, wxT("Toggle me!"), wxPoint(170, 785), wxSize(25, 25), 0);
						break;
					case 14:
						GuiEditor::Get()->AddWidget<wxSearchCtrl>(panel, wxID_ANY, wxEmptyString, wxPoint(0, 70), wxDefaultSize, 0);
						break;
					case 15:
						GuiEditor::Get()->AddWidget<wxFontPickerCtrl>(panel, wxID_ANY, wxNullFont, wxPoint(0, 70), wxDefaultSize, wxFNTP_DEFAULT_STYLE);
						break;
					case 16:
						GuiEditor::Get()->AddWidget<wxFilePickerCtrl>(panel, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxPoint(0, 70), wxDefaultSize, wxFLP_DEFAULT_STYLE);
						break;
					case 17:
						GuiEditor::Get()->AddWidget<wxDirPickerCtrl>(panel, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxPoint(0, 70), wxDefaultSize, wxDIRP_DEFAULT_STYLE);
						break;
					case 18:
						GuiEditor::Get()->AddWidget<wxDatePickerCtrl>(panel, wxID_ANY, wxDefaultDateTime, wxPoint(0, 70), wxDefaultSize, wxDP_DEFAULT);
						break;
					case 19:
						GuiEditor::Get()->AddWidget<wxTimePickerCtrl>(panel, wxID_ANY, wxDefaultDateTime, wxPoint(0, 70), wxDefaultSize, wxTP_DEFAULT);
						break;
					case 20:
						GuiEditor::Get()->AddWidget<wxCalendarCtrl>(panel, wxID_ANY, wxDefaultDateTime, wxPoint(0, 70), wxDefaultSize, wxCAL_SHOW_HOLIDAYS);
						break;
					case 21:
						GuiEditor::Get()->AddWidget<wxGenericDirCtrl>(panel, wxID_ANY, wxEmptyString, wxPoint(0, 70), wxDefaultSize, wxDIRCTRL_3D_INTERNAL | wxSUNKEN_BORDER, wxEmptyString, 0);
						break;
					case 22:
						GuiEditor::Get()->AddWidget<wxSpinButton>(panel, wxID_ANY, wxPoint(0, 70), wxDefaultSize, 0);
						break;
				}
			}
			break;
		}
		case 68: /* D */
		{
			DuplicateWidget();
			break;
		}
		case 127: /* Delete */
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				widgets.erase(m_SelectedWidget);
				text->Destroy();
			}
			break;
		}
		case 315: /* Up*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxPoint pos = text->GetPosition();
				pos.y -= m_speed;
				text->SetPosition(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 317: /* Down*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxPoint pos = text->GetPosition();
				pos.y += m_speed;
				text->SetPosition(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 314: /* Left*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxPoint pos = text->GetPosition();
				pos.x -= m_speed;
				text->SetPosition(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 316: /* Right*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxPoint pos = text->GetPosition();
				pos.x += m_speed;
				text->SetPosition(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 330: /* 6*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxSize pos = text->GetSize();
				pos.x += m_speed;
				text->SetSize(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 332: /* 8*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxSize pos = text->GetSize();
				pos.y += m_speed;
				text->SetSize(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 328: /* 4*/
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxSize pos = text->GetSize();
				pos.x -= m_speed;
				text->SetSize(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
		case 326: /* 2 */
		{
			if(t != nullptr)
			{
				wxStaticText* text = reinterpret_cast<wxStaticText*>(m_SelectedWidget);
				wxSize pos = text->GetSize();
				pos.y -= m_speed;
				text->SetSize(std::move(pos));
				//m_propgrid->Update(std::make_pair((void*)text, t));
			}
			break;
		}
	}
}

Widget* GuiEditor::FindwxText(wxObject* object_to_find)
{
	Widget* a = nullptr;
	if(m_SelectedWidget != nullptr || object_to_find)
	{
		try
		{
			a = widgets[object_to_find == nullptr ? m_SelectedWidget : object_to_find];
		}
		catch(...) {}
	}
	return a;
}

void GuiEditor::MarkSelectedItem()
{
	wxClientDC dc(panel);
	Widget* t = FindwxText();
	if(t != nullptr)
	{
		wxPoint pos = ((wxStaticText*)m_SelectedWidget)->GetPosition();
		wxSize size = ((wxStaticText*)m_SelectedWidget)->GetSize();
		dc.Clear();
		dc.SetPen(wxPen(*wxRED, 1));
		dc.DrawRectangle(pos.x - 1, pos.y - 1, size.x + 2, size.y + 2);
	}
}

void GuiEditor::OnOpen()
{
	LoadWidgets();
}

void GuiEditor::OnPaint()
{
	MarkSelectedItem();
}

void GuiEditor::OnSave()
{
	if(file_path.IsEmpty())
	{
		wxFileDialog saveFileDialog(panel, _("Save XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if(saveFileDialog.ShowModal() == wxID_CANCEL)
			return;
		file_path = saveFileDialog.GetPath();
	}
	SaveWidgets();
}

void GuiEditor::OnSaveAs()
{
	wxFileDialog saveFileDialog(panel, _("Save XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if(saveFileDialog.ShowModal() == wxID_CANCEL)
		return;
	file_path = saveFileDialog.GetPath();
	SaveWidgets();
}

void GuiEditor::OnDestroyAll()
{
	for(auto x : widgets)
	{
		if(!x.first || !x.second) continue;
		wxStaticText* t = reinterpret_cast<wxStaticText*>(x.first);
		t->Destroy();
		delete x.second;
	}
	widgets.clear();
}

void GuiEditor::DuplicateWidget()
{
	auto x = widgets.find(m_SelectedWidget);
	if(x != widgets.end())
	{
		if(x->second->type == typeid(wxButton*).hash_code())
		{
			wxButton* t = dynamic_cast<wxButton*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxComboBox*).hash_code())
		{
			wxComboBox* t = dynamic_cast<wxComboBox*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize());
		}
		else if(x->second->type == typeid(wxChoice*).hash_code())
		{
			wxChoice* t = dynamic_cast<wxChoice*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetPosition(), t->GetSize(), t->GetStrings(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxListBox*).hash_code())
		{
			wxListBox* t = dynamic_cast<wxListBox*>(x->first);
			wxArrayString m_choiceChoices2;
			m_choiceChoices2.Add("First");
			m_choiceChoices2.Add("Second");
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetPosition(), t->GetSize(), m_choiceChoices2, t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxCheckBox*).hash_code())
		{
			wxCheckBox* t = dynamic_cast<wxCheckBox*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxRadioButton*).hash_code())
		{
			wxRadioButton* t = dynamic_cast<wxRadioButton*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxStaticLine*).hash_code())
		{
			wxStaticLine* t = dynamic_cast<wxStaticLine*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxSlider*).hash_code())
		{
			wxSlider* t = dynamic_cast<wxSlider*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetValue(), t->GetMin(), t->GetMax(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxGauge*).hash_code())
		{
			wxGauge* t = dynamic_cast<wxGauge*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetRange(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxStaticText*).hash_code())
		{
			wxStaticText* t = dynamic_cast<wxStaticText*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxSpinCtrl*).hash_code())
		{
			wxSpinCtrl* t = dynamic_cast<wxSpinCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxSpinCtrlDouble*).hash_code())
		{
			wxSpinCtrlDouble* t = dynamic_cast<wxSpinCtrlDouble*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxTextCtrl*).hash_code())
		{
			wxTextCtrl* t = dynamic_cast<wxTextCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxToggleButton*).hash_code())
		{
			wxToggleButton* t = dynamic_cast<wxToggleButton*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxSearchCtrl*).hash_code())
		{
			wxSearchCtrl* t = dynamic_cast<wxSearchCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetLabelText(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxFontPickerCtrl*).hash_code())
		{
			wxFontPickerCtrl* t = dynamic_cast<wxFontPickerCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetFont(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxFilePickerCtrl*).hash_code())
		{
			wxFilePickerCtrl* t = dynamic_cast<wxFilePickerCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxDirPickerCtrl*).hash_code())
		{
			wxDirPickerCtrl* t = dynamic_cast<wxDirPickerCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, wxEmptyString, wxT("Select a file"), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxDatePickerCtrl*).hash_code())
		{
			wxDatePickerCtrl* t = dynamic_cast<wxDatePickerCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetValue(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxTimePickerCtrl*).hash_code())
		{
			wxTimePickerCtrl* t = dynamic_cast<wxTimePickerCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetValue(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxCalendarCtrl*).hash_code())
		{
			wxCalendarCtrl* t = dynamic_cast<wxCalendarCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetDate(), t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxGenericDirCtrl*).hash_code())
		{
			wxGenericDirCtrl* t = dynamic_cast<wxGenericDirCtrl*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, wxEmptyString, t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag());
		}
		else if(x->second->type == typeid(wxSpinButton*).hash_code())
		{
			wxSpinButton* t = dynamic_cast<wxSpinButton*>(x->first);
			GuiEditor::Get()->AddWidget<std::decay<decltype(*t)>::type>(panel, wxID_ANY, t->GetPosition(), t->GetSize(), t->GetWindowStyleFlag(), t->GetName());
		}
	}
}

void ParseDefaultWidgetFormat(const boost::property_tree::ptree::value_type& v, wxPoint& pos, wxSize& size, wxSize& min_size, wxSize& max_size,
	int& fontsize, int& fontfamily, int& fontstyle, int& fontweight, int& is_underlined, char* fontname, uint8_t fg_colors[3], uint8_t bg_colors[3])
{
	sscanf(v.second.get<std::string>("<xmlattr>.pos").c_str(), "%d,%d", &pos.x, &pos.y);
	sscanf(v.second.get<std::string>("<xmlattr>.size").c_str(), "%d,%d", &size.x, &size.y);
	sscanf(v.second.get<std::string>("<xmlattr>.min_size").c_str(), "%d,%d", &min_size.x, &min_size.y);
	sscanf(v.second.get<std::string>("<xmlattr>.max_size").c_str(), "%d,%d", &max_size.x, &max_size.y);
	sscanf(v.second.get<std::string>("<xmlattr>.fg_color").c_str(), "%hhd,%hhd,%hhd", &fg_colors[0], &fg_colors[1], &fg_colors[2]);
	sscanf(v.second.get<std::string>("<xmlattr>.bg_color").c_str(), "%hhd,%hhd,%hhd", &bg_colors[0], &bg_colors[1], &bg_colors[2]);
	sscanf(v.second.get<std::string>("<xmlattr>.font").c_str(), "%d,%d,%d,%d,%d,%[^\n]s", &fontsize, &fontfamily, &fontstyle, &fontweight, &is_underlined, fontname);
}

template <typename T> void SetDefaultWidgetFormat(T w, std::string&& tooltip, wxPoint& pos, wxSize& size, wxSize& min_size, wxSize& max_size,
	int& fontsize, int& fontfamily, int& fontstyle, int& fontweight, int& is_underlined, char* fontname, uint8_t fg_colors[3], uint8_t bg_colors[3])
{
	w->SetToolTip(tooltip);
	w->SetMinSize(min_size);
	w->SetMaxSize(max_size);
	w->SetForegroundColour(wxColour(fg_colors[0], fg_colors[1], fg_colors[2]));
	w->SetBackgroundColour(wxColour(bg_colors[0], bg_colors[1], bg_colors[2]));
	w->SetFont(wxFont(fontsize, static_cast<wxFontFamily>(fontfamily), static_cast<wxFontStyle>(fontstyle), static_cast<wxFontWeight>(fontweight), is_underlined != 0, wxString(fontname)));
}

void GuiEditor::LoadWidgets()
{
	const std::string filename = "./wx.xml";

	// Create an empty property tree object
	using boost::property_tree::ptree;
	ptree pt;

	read_xml(filename, pt);

	for(const ptree::value_type& v : pt)
	{
		DBG("%s\n", v.first.c_str());
	}
	std::cout << std::endl;

	for(const ptree::value_type& v : pt.get_child("wxCreatorXmlFile"))
	{
		wxPoint pos;
		wxSize size, min_size, max_size;
		int fontsize, fontfamily, fontstyle, fontweight, is_underlined;
		char fontname[128];
		std::string type = v.second.get<std::string>("<xmlattr>.type");
		if(type == "button")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
			wxButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "combobox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxArrayString a;
			wxComboBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, a, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "choise")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxArrayString a;
			wxChoice* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, a, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "listbox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxListBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "checkbox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxCheckBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "radiobutton")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxRadioButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "staticline")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxStaticLine* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "slider")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSlider* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, std::stoi(v.second.get<std::string>("<xmlattr>.value")), std::stoi(v.second.get<std::string>("<xmlattr>.min")),
				std::stoi(v.second.get<std::string>("<xmlattr>.max")), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "gauge")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxGauge* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, std::stoi(v.second.get<std::string>("<xmlattr>.max")), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "spincontrol")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSpinCtrl* tmp = AddWidget<wxSpinCtrl>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")),
				std::stoi(v.second.get<std::string>("<xmlattr>.min")), std::stoi(v.second.get<std::string>("<xmlattr>.max")), std::stoi(v.second.get<std::string>("<xmlattr>.value")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "spincontroldouble")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSpinCtrlDouble* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")),
				std::stoi(v.second.get<std::string>("<xmlattr>.min")), std::stoi(v.second.get<std::string>("<xmlattr>.max")), std::stoi(v.second.get<std::string>("<xmlattr>.value")), 
				std::stoi(v.second.get<std::string>("<xmlattr>.inc")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "textcontrol")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxTextCtrl* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "togglebutton")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxToggleButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "statictext")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxStaticText* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, std::stoi(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
	}
}

void GuiEditor::SaveWidgets()
{
	wxFile file;
	bool status = file.Open(file_path, wxFile::OpenMode::write);
	if(!status)
		return;
	wxString lines = "<wxCreatorXmlFile>\n";
	for(auto& x : widgets)
	{
		if(!x.first || !x.second) continue;
		if(x.second->type == typeid(wxButton*).hash_code())
		{
			wxButton* t = dynamic_cast<wxButton*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"button\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxComboBox*).hash_code())
		{
			wxComboBox* t = dynamic_cast<wxComboBox*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"combobox\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxChoice*).hash_code())
		{
			wxChoice* t = dynamic_cast<wxChoice*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"choice\" id=\"-1\" name=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxListBox*).hash_code())
		{
			wxListBox* t = dynamic_cast<wxListBox*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"choice\" id=\"-1\" name=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxCheckBox*).hash_code())
		{
			wxCheckBox* t = dynamic_cast<wxCheckBox*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"combobox\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxRadioButton*).hash_code())
		{
			wxRadioButton* t = dynamic_cast<wxRadioButton*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"combobox\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxStaticLine*).hash_code())
		{
			wxStaticLine* t = dynamic_cast<wxStaticLine*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"staticline\" id=\"-1\" name=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxSlider*).hash_code())
		{
			wxSlider* t = dynamic_cast<wxSlider*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"slider\" id=\"-1\" name=\"%s\" tooltip=\"%s\" value=\"%d\" min=\"%d\" max=\"%d\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(), t->GetValue(), t->GetMin(), t->GetMax(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxGauge*).hash_code())
		{
			wxGauge* t = dynamic_cast<wxGauge*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"gauge\" id=\"-1\" name=\"%s\" tooltip=\"%s\" value=\"%d\" max=\"%d\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(), t->GetValue(), 100,
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxSpinCtrl*).hash_code())
		{
			wxSpinCtrl* t = dynamic_cast<wxSpinCtrl*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"spincontrol\" id=\"-1\" name=\"%s\" tooltip=\"%s\" value=\"%d\" min=\"%d\" max=\"%d\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(), t->GetValue(), t->GetMin(), t->GetMax(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxSpinCtrlDouble*).hash_code())
		{
			wxSpinCtrlDouble* t = dynamic_cast<wxSpinCtrlDouble*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"spincontroldouble\" id=\"-1\" name=\"%s\" tooltip=\"%s\" value=\"%d\" min=\"%d\" max=\"%d\" inc=\"%d\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetToolTipText(), t->GetValue(), t->GetMin(), t->GetMax(), t->GetIncrement(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxTextCtrl*).hash_code())
		{
			wxTextCtrl* t = dynamic_cast<wxTextCtrl*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"textcontrol\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxToggleButton*).hash_code())
		{
			wxToggleButton* t = dynamic_cast<wxToggleButton*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"togglebutton\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
		else if(x.second->type == typeid(wxStaticText*).hash_code())
		{
			wxStaticText* t = dynamic_cast<wxStaticText*>(x.first);
			wxFont font = t->GetFont();
			lines += wxString::Format("\t<widget type=\"togglebutton\" id=\"-1\" name=\"%s\" label=\"%s\" tooltip=\"%s\" pos=\"%d,%d\" size=\"%d,%d\" min_size=\"%d,%d\" max_size=\"%d,%d\"\
 fg_color=\"%d,%d,%d\" bg_color=\"%d,%d,%d\" font=\"%d,%d,%d,%d,%d,%s\" flags=\"%d\" />\n", x.second->name, t->GetLabelText(), t->GetToolTipText(),
				t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, t->GetMinSize().x, t->GetMinSize().y, t->GetMaxSize().x, t->GetMaxSize().y,
				t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue(),
				t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue(), font.GetPointSize(), font.GetFamily(), font.GetStyle(), font.GetWeight(),
				font.GetUnderlined(), font.GetFaceName(), t->GetWindowStyleFlag());
		}
	}
	lines += "</wxCreatorXmlFile>\n";
	file.Write(lines);
	file.Close();
}

void GuiEditor::AddFlags(wxString& wxstr, void* widget, Widget* obj, const long* to_pointer, const wxString* to_pointer_str, const long max_array_size)
{
	wxStaticText* t = reinterpret_cast<wxStaticText*>(widget);
	int counter = 0;
	for(uint8_t i = 0; i != max_array_size; i++)
	{
		if(t->GetWindowStyleFlag() & to_pointer[i])
		{
			if(counter)
				wxstr += " | ";
			wxstr += to_pointer_str[i];
			counter++;
		}
	}
	if(wxstr.IsEmpty())
		wxstr = "0";
}

void GuiEditor::AddFontAndColor(wxString& wxstr, void* widget, Widget* obj)
{
	wxStaticText* t = reinterpret_cast<wxStaticText*>(widget);
	wxString tooltip = t->GetToolTipText();
	if(!tooltip.IsEmpty())
		wxstr += wxString::Format("%s->SetToolTip(\"%s\");\n", obj->name, tooltip);
	if(obj->fg_color_changed)
		wxstr += wxString::Format("%s->SetForegroundColour(wxColour(%d, %d, %d));\n",
			obj->name, t->GetForegroundColour().Red(), t->GetForegroundColour().Green(), t->GetForegroundColour().Blue());
	if(obj->bg_color_changed)
		wxstr += wxString::Format("%s->SetBackgroundColour(wxColour(%d, %d, %d));\n",
			obj->name, t->GetBackgroundColour().Red(), t->GetBackgroundColour().Green(), t->GetBackgroundColour().Blue());
	wxSize min_size = t->GetMinSize();
	if(min_size.x != -1 || min_size.y != -1)
		wxstr += wxString::Format("%s->SetMinSize(wxPoint(%d, %d));\n", obj->name, min_size.x, min_size.y);
	wxSize max_size = t->GetMaxSize();
	if(max_size.x != -1 || max_size.y != -1)
		wxstr += wxString::Format("%s->SetMaxSize(wxPoint(%d, %d));\n", obj->name, max_size.x, max_size.y);
	wxFont font = t->GetFont();
	wxString strFontPointSize;
	if(font.GetPointSize() != -1)
		strFontPointSize = wxString::Format("%d", font.GetPointSize());
	else
		strFontPointSize = wxString("wxNORMAL_FONT->GetPointSize()");
	if(font.GetFamily() != wxFONTFAMILY_DEFAULT || !font.GetFaceName().IsEmpty() || font.GetStyle() != wxFONTSTYLE_NORMAL || font.GetWeight() != wxFONTWEIGHT_NORMAL
		|| font.GetUnderlined())
	{
		/*
		wxFontWeight a = font.GetWeight();
		wxstr += wxString::Format("%s->SetFont( wxFont(%s, %s, %s, %s, %s, wxT(\"%s\")));\n",
			obj->name, strFontPointSize, GetNameFromEnum<wxFontFamily>(font.GetFamily()), GetNameFromEnum<wxFontStyle>(font.GetStyle()),
			GetNameFromEnum<wxFontWeight>(a),
			font.GetUnderlined() ? wxString("true") : wxString("false"), font.GetFaceName());
		Sleep(10);
		*/
	}
	wxstr += "\n";
}
/*
#include "utils/magic_enum.hpp"

template <class T> wxString GetNameFromEnum(T to_get)
{
	char ret[128];
	auto color_name = magic_enum::enum_name(to_get);
	memcpy(ret, color_name.data(), color_name.length());
	ret[color_name.length()] = 0;
	wxString str(ret);
	return str;
}
*/
void GuiEditor::GenerateCode(wxString& str)
{
#define FLAG(val) val##_style_values, val##_style_flags, WXSIZEOF(val##_style_flags)
	for(auto& x : widgets)
	{
		if(!x.first || !x.second) continue;
		if(x.second->type == typeid(wxButton*).hash_code())
		{
			wxButton* t = dynamic_cast<wxButton*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(button));
			str += wxString::Format("%s = new wxButton(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
			/*
			wxString wxstr;
			wxFont font = t->GetFont();
			wxString strFontPointSize;
			wxFontWeight a = font.GetWeight();
			wxstr += wxString::Format("a->SetFont( wxFont(%s, %s, %s, ));\n",
				GetNameFromEnum<wxFontFamily>(font.GetFamily()), GetNameFromEnum<wxFontStyle>(font.GetStyle()),
				GetNameFromEnum<wxFontWeight>(font.GetWeight()));
			DBG("a");*/
		}
		else if(x.second->type == typeid(wxComboBox*).hash_code())
		{
			wxComboBox* t = dynamic_cast<wxComboBox*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(combobox));
			str += wxString::Format("%s = new wxComboBox(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
		}
		else if(x.second->type == typeid(wxChoice*).hash_code())
		{
			wxChoice* t = dynamic_cast<wxChoice*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(choice));
			str += wxString::Format("%s = new wxChoice(this, wxID_ANY, wxPoint(%d, %d), wxSize(%d, %d), m_choiceChoices, %s);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			str += wxString::Format("%s->SetSelection(0);\n", x.second->name);
		}
		else if(x.second->type == typeid(wxListBox*).hash_code())
		{
			wxListBox* t = dynamic_cast<wxListBox*>(x.first);
			str += wxString::Format("%s = new wxListBox(this, wxID_ANY, wxPoint(%d, %d), wxSize(%d, %d));\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxCheckBox*).hash_code())
		{
			wxCheckBox* t = dynamic_cast<wxCheckBox*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(checkbox));
			str += wxString::Format("%s = new wxCheckBox(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
		}

		else if(x.second->type == typeid(wxStaticLine*).hash_code())
		{
			wxStaticLine* t = dynamic_cast<wxStaticLine*>(x.first);
			//wxString flagsStr;
			//AddFlags(flagsStr, x.first, x.second, FLAG(checkbox));
			str += wxString::Format("%s = new wxStaticLine(this, wxID_ANY, wxPoint(%d, %d), wxSize(%d, %d), 0);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxSlider*).hash_code())
		{
			wxSlider* t = dynamic_cast<wxSlider*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(slider));
			str += wxString::Format("%s = new wxSlider(this, wxID_ANY, %d, %d, %d, wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, 0, t->GetMin(), t->GetMax(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxGauge*).hash_code())
		{
			wxGauge* t = dynamic_cast<wxGauge*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(gauge));
			str += wxString::Format("%s = new wxGauge(this, wxID_ANY, 100, wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxSpinCtrl*).hash_code())
		{
			wxSpinCtrl* t = dynamic_cast<wxSpinCtrl*>(x.first);
			str += wxString::Format("%s = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxPoint(%d, %d), wxSize(%d, %d), wxSP_ARROW_KEYS, 1, 255, 1);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxTextCtrl*).hash_code())
		{
			wxTextCtrl* t = dynamic_cast<wxTextCtrl*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(textctrl));
			str += wxString::Format("%s = new wxTextCtrl(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxToggleButton*).hash_code())
		{
			wxToggleButton* t = dynamic_cast<wxToggleButton*>(x.first);
			str += wxString::Format("%s = new wxToggleButton(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), 0);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxSpinCtrlDouble*).hash_code())
		{
			wxSpinCtrlDouble* t = dynamic_cast<wxSpinCtrlDouble*>(x.first);
			str += wxString::Format("%s = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxPoint(%d, %d), wxSize(%d, %d), wxSP_ARROW_KEYS, 0, 100, 0, 1);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxSearchCtrl*).hash_code())
		{
			wxSearchCtrl* t = dynamic_cast<wxSearchCtrl*>(x.first);
			str += wxString::Format("%s = new wxSearchCtrl(this, wxID_ANY, wxEmptyString, wxPoint(%d, %d), wxSize(%d, %d), 0);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxFontPickerCtrl*).hash_code())
		{
			wxFontPickerCtrl* t = dynamic_cast<wxFontPickerCtrl*>(x.first);
			str += wxString::Format("%s = new wxFontPickerCtrl(this, wxID_ANY, wxNullFont, xPoint(%d, %d), wxSize(%d, %d), wxFNTP_DEFAULT_STYLE);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxFilePickerCtrl*).hash_code())
		{
			wxFilePickerCtrl* t = dynamic_cast<wxFilePickerCtrl*>(x.first);
			str += wxString::Format("%s = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxT(\"Select a file\"), wxT(\"*.*\"), wxPoint(%d, %d), wxSize(%d, %d), wxFLP_DEFAULT_STYLE);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxDirPickerCtrl*).hash_code())
		{
			wxDirPickerCtrl* t = dynamic_cast<wxDirPickerCtrl*>(x.first);
			str += wxString::Format("%s = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, wxT(\"Select a folder\"), wxPoint(%d, %d), wxSize(%d, %d), wxDIRP_DEFAULT_STYLE);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxTimePickerCtrl*).hash_code())
		{
			wxTimePickerCtrl* t = dynamic_cast<wxTimePickerCtrl*>(x.first);
			str += wxString::Format("%s = new wxTimePickerCtrl(this, wxID_ANY, wxDefaultDateTime, wxPoint(%d, %d), wxSize(%d, %d), wxTP_DEFAULT);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxCalendarCtrl*).hash_code())
		{
			wxCalendarCtrl* t = dynamic_cast<wxCalendarCtrl*>(x.first);
			str += wxString::Format("%s = new wxCalendarCtrl(this, wxID_ANY, wxDefaultDateTime, wxPoint(%d, %d), wxSize(%d, %d), wxCAL_SHOW_HOLIDAYS);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxGenericDirCtrl*).hash_code())
		{
			wxGenericDirCtrl* t = dynamic_cast<wxGenericDirCtrl*>(x.first);
			str += wxString::Format("%s = new wxGenericDirCtrl(this, wxID_ANY, wxEmptyString, wxPoint(%d, %d), wxSize(%d, %d), wxDIRCTRL_3D_INTERNAL | wxSUNKEN_BORDER, wxEmptyString, 0);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxSpinButton*).hash_code())
		{
			wxSpinButton* t = dynamic_cast<wxSpinButton*>(x.first);
			str += wxString::Format("%s = new wxSpinButton(this, wxID_ANY, wxPoint(%d, %d), wxSize(%d, %d), 0);\n",
				x.second->name, t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y);
			AddFontAndColor(str, x.first, x.second);
		}
	}
}
