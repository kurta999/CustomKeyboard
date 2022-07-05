#include "pch.hpp"

/* This is an updated code of my old project called wxCreator - this file doesn't really follow OO principles :) */

const wxString choices[] = { "wxButton", "wxComboBox", "wxChoise", "wxListBox", "wxCheckBox", "wxRadioButton", "wxStaticLine", "wxSlider", "wxGauge", "wxStaticText",
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

void GuiEditor::Init(GuiEditorMain* panel_)
{
	panel = panel_;
	AddFlagsToPropgrid();
}

void GuiEditor::OnClick(void* object)
{
	Widget* t = FindwxText((wxObject*)object);
	if(t != nullptr)
	{
		m_SelectedWidget = (wxObject*)object;
		MarkSelectedItem();
		panel->UpdatePropgrid({ t->id, &t->name, dynamic_cast<wxWindow*>(m_SelectedWidget) });
	}
}

void GuiEditor::OnMouseMotion(wxPoint& pos)
{
	Widget* t = FindwxText();
	if(t != nullptr)
	{
		dynamic_cast<wxWindow*>(m_SelectedWidget)->SetPosition(pos);
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
			LOG(LogLevel::Notification, "Move speed has increased to {}.\n", ++m_speed);
			break;
		}
		case 390: /* - */
		{
			if(!--m_speed)
				m_speed = 1;
			LOG(LogLevel::Notification, "Move speed has decresed to {}.\n", m_speed);
			break;
		}
		case 'A': /* A */
		{
			wxSingleChoiceDialog s(panel, "Select widget from list", "Add widget", WXSIZEOF(choices), choices);
			if(s.ShowModal() == wxID_OK)
			{
				wxPoint DefaultPos = wxPoint(0, 20);
				int sel = s.GetSelection();
				switch(sel)
				{
					case 0:
						GuiEditor::Get()->AddWidget<wxButton>(panel, wxID_ANY, wxT("btn"), DefaultPos, wxSize(25, 25), 0);
						break;
					case 1:
						GuiEditor::Get()->AddWidget<wxComboBox>(panel, wxID_ANY, wxT("combo"), DefaultPos, wxSize(25, 25), 0);
						break;
					case 2:
					{
						wxArrayString m_choiceChoices2;
						m_choiceChoices2.Add("First");
						m_choiceChoices2.Add("Second");
						GuiEditor::Get()->AddWidget<wxChoice>(panel, wxID_ANY, DefaultPos, wxSize(40, 25), m_choiceChoices2, 0);
						break;
					}
					case 3:
						GuiEditor::Get()->AddWidget<wxListBox>(panel, wxID_ANY, DefaultPos, wxDefaultSize, 0);
						break;
					case 4:
						GuiEditor::Get()->AddWidget<wxCheckBox>(panel, wxID_ANY, wxT("Check Me!"), DefaultPos, wxDefaultSize, 0);
						break;
					case 5:
						GuiEditor::Get()->AddWidget<wxRadioButton>(panel, wxID_ANY, wxT("RadioBtn"), DefaultPos, wxDefaultSize, 0);
						break;
					case 6:
						GuiEditor::Get()->AddWidget<wxStaticLine>(panel, wxID_ANY, DefaultPos, wxSize(25, 25), 0);
						break;
					case 7:
						GuiEditor::Get()->AddWidget<wxSlider>(panel, wxID_ANY, 50, 0, 100, DefaultPos, wxSize(25, 25), 0);
						break;
					case 8:
						GuiEditor::Get()->AddWidget<wxGauge>(panel, wxID_ANY, 100, DefaultPos, wxSize(25, 25), 0);
						break;
					case 9:
						GuiEditor::Get()->AddWidget<wxStaticText>(panel, wxID_ANY, wxT("wxStaticText"), DefaultPos, wxDefaultSize, 0);
						break;
					case 10:
						GuiEditor::Get()->AddWidget<wxSpinCtrl>(panel, wxID_ANY, wxEmptyString, DefaultPos, wxSize(25, 25), wxSP_ARROW_KEYS, 1, 255, 1);
						break;
					case 11:
						GuiEditor::Get()->AddWidget<wxSpinCtrlDouble>(panel, wxID_ANY, wxEmptyString, DefaultPos, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);
						break;
					case 12:
						GuiEditor::Get()->AddWidget<wxTextCtrl>(panel, wxID_ANY, wxT("wxTextCtrl"), DefaultPos, wxSize(25, 25), 0);
						break;
					case 13:
						GuiEditor::Get()->AddWidget<wxToggleButton>(panel, wxID_ANY, wxT("Toggle me!"), DefaultPos, wxSize(25, 25), 0);
						break;
					case 14:
						GuiEditor::Get()->AddWidget<wxSearchCtrl>(panel, wxID_ANY, wxEmptyString, DefaultPos, wxDefaultSize, 0);
						break;
					case 15:
						GuiEditor::Get()->AddWidget<wxFontPickerCtrl>(panel, wxID_ANY, wxNullFont, DefaultPos, wxDefaultSize, wxFNTP_DEFAULT_STYLE);
						break;
					case 16:
						GuiEditor::Get()->AddWidget<wxFilePickerCtrl>(panel, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), DefaultPos, wxDefaultSize, wxFLP_DEFAULT_STYLE);
						break;
					case 17:
						GuiEditor::Get()->AddWidget<wxDirPickerCtrl>(panel, wxID_ANY, wxEmptyString, wxT("Select a folder"), DefaultPos, wxDefaultSize, wxDIRP_DEFAULT_STYLE);
						break;
					case 18:
						GuiEditor::Get()->AddWidget<wxDatePickerCtrl>(panel, wxID_ANY, wxDefaultDateTime, DefaultPos, wxDefaultSize, wxDP_DEFAULT);
						break;
					case 19:
						GuiEditor::Get()->AddWidget<wxTimePickerCtrl>(panel, wxID_ANY, wxDefaultDateTime, DefaultPos, wxDefaultSize, wxTP_DEFAULT);
						break;
					case 20:
						GuiEditor::Get()->AddWidget<wxCalendarCtrl>(panel, wxID_ANY, wxDefaultDateTime, DefaultPos, wxDefaultSize, wxCAL_SHOW_HOLIDAYS);
						break;
					case 21:
						GuiEditor::Get()->AddWidget<wxGenericDirCtrl>(panel, wxID_ANY, wxEmptyString, DefaultPos, wxDefaultSize, wxDIRCTRL_3D_INTERNAL | wxSUNKEN_BORDER, wxEmptyString, 0);
						break;
					case 22:
						GuiEditor::Get()->AddWidget<wxSpinButton>(panel, wxID_ANY, DefaultPos, wxDefaultSize, 0);
						break;
				}
			}
			break;
		}
		case 'D': /* D */
		{
			DuplicateWidget();
			break;
		}
		case 127: /* Delete */
		{
			if(t != nullptr)
			{
				wxWindow* w = dynamic_cast<decltype(w)>(m_SelectedWidget);
				widgets.erase(m_SelectedWidget);
				w->Destroy();
			}
			break;
		}
		case 315: /* Up*/
		{
			if(t != nullptr)
			{
				AdjustWidgetPos({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, std::numeric_limits<int8_t>::max(), -1);
			}
			break;
		}
		case 317: /* Down*/
		{
			if(t != nullptr)
			{
				AdjustWidgetPos({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, std::numeric_limits<int8_t>::max(), 1);
			}
			break;
		}
		case 314: /* Left*/
		{
			if(t != nullptr)
			{
				AdjustWidgetPos({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, -1, std::numeric_limits<int8_t>::max());
			}
			break;
		}
		case 316: /* Right*/
		{
			if(t != nullptr)
			{
				AdjustWidgetPos({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, 1, std::numeric_limits<int8_t>::max());
			}
			break;
		}
		case 330: /* 6*/
		{
			if(t != nullptr)
			{
				AdjustWidgetSize({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, 1, std::numeric_limits<int8_t>::max());
			}
			break;
		}
		case 332: /* 8*/
		{
			if(t != nullptr)
			{
				AdjustWidgetSize({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, std::numeric_limits<int8_t>::max(), 1);
			}
			break;
		}
		case 328: /* 4*/
		{
			if(t != nullptr)
			{
				AdjustWidgetSize({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, -1, std::numeric_limits<int8_t>::max());
			}
			break;
		}
		case 326: /* 2 */
		{
			if(t != nullptr)
			{
				AdjustWidgetSize({ dynamic_cast<wxWindow*>(m_SelectedWidget), t }, std::numeric_limits<int8_t>::max(), -1);
			}
			break;
		}
	}
}

void GuiEditor::OnPropertyGridChange(wxPGProperty* prop)
{
	Widget* t = FindwxText();
	if(prop == panel->m_pgName)
	{
		if(t != nullptr)
		{
			wxString str;
			prop->GetValue().Convert(&str);
			t->name = std::move(str);
		}
	}
	else if(prop == panel->m_pgLabel)
	{
		if(t != nullptr)
		{
			wxString str;
			prop->GetValue().Convert(&str);
			dynamic_cast<wxControl*>(m_SelectedWidget)->SetLabelText(str);
		}
	}	
	else if(prop == panel->m_pgPos)
	{
		if(t != nullptr)
		{
			int x, y;
			wxString str;
			prop->GetValue().Convert(&str);
			if(sscanf(str.c_str(), "%d,%d", &x, &y) == 2)
				dynamic_cast<wxWindow*>(m_SelectedWidget)->SetPosition(wxPoint(x, y));
		}
	}	
	else if(prop == panel->m_pgSize)
	{
		if(t != nullptr)
		{
			int x, y;
			wxString str;
			prop->GetValue().Convert(&str);
			if(sscanf(str.c_str(), "%d,%d", &x, &y) == 2)
				dynamic_cast<wxWindow*>(m_SelectedWidget)->SetSize(wxSize(x, y));
		}
	}
	else if(prop == panel->m_pgMinSize)
	{
		if(t != nullptr)
		{
			int x, y;
			wxString str;
			prop->GetValue().Convert(&str);
			if(sscanf(str.c_str(), "%d,%d", &x, &y) == 2)
				dynamic_cast<wxWindow*>(m_SelectedWidget)->SetMinSize(wxSize(x, y));
		}
	}
	else if(prop == panel->m_pgMaxSize)
	{
		if(t != nullptr)
		{
			int x, y;
			wxString str;
			prop->GetValue().Convert(&str);
			if(sscanf(str.c_str(), "%d,%d", &x, &y) == 2)
				dynamic_cast<wxWindow*>(m_SelectedWidget)->SetMaxSize(wxSize(x, y));
		}
	}	
	else if(prop == panel->m_pgForegroundColor)
	{
		if(t != nullptr)
		{
			wxVariant a = prop->GetValue();
			wxColour color;
			color << a;
			dynamic_cast<wxWindow*>(m_SelectedWidget)->SetForegroundColour(color);
			dynamic_cast<wxWindow*>(m_SelectedWidget)->Refresh();
			t->fg_color_changed = 1;
		}
	}
	else if(prop == panel->m_pgBackgroundColor)
	{
		if(t != nullptr)
		{
			wxVariant a = prop->GetValue();
			wxColour color;
			color << a;
			dynamic_cast<wxWindow*>(m_SelectedWidget)->SetBackgroundColour(color);
			dynamic_cast<wxWindow*>(m_SelectedWidget)->Refresh();
			t->bg_color_changed = 1;
		}
	}
	else if(prop == panel->m_pgFont)
	{
		if(t != nullptr)
		{
			wxVariant a = prop->GetValue();
			wxFont fnt;
			fnt << a;
			dynamic_cast<wxWindow*>(m_SelectedWidget)->SetFont(fnt);
			t->font_changed = 1;
		}
	}
	else if(prop == panel->m_pgTooltip)
	{
		if(t != nullptr)
		{
			wxString str;
			prop->GetValue().Convert(&str);
			dynamic_cast<wxWindow*>(m_SelectedWidget)->SetToolTip(str);
		}
	}	
	else if(prop == panel->m_pgEnabled)
	{
		if(t != nullptr)
		{
			bool is_enabled;
			prop->GetValue().Convert(&is_enabled);
			dynamic_cast<wxWindow*>(m_SelectedWidget)->Enable(&is_enabled);
		}
	}
	else if(prop == panel->m_pgHidden)
	{
		if(t != nullptr)
		{
			bool is_hide;
			prop->GetValue().Convert(&is_hide);
			if(is_hide)
				dynamic_cast<wxWindow*>(m_SelectedWidget)->Hide();
			else
				dynamic_cast<wxWindow*>(m_SelectedWidget)->Show();
		}
	}
	else if(prop == panel->m_pgButtonStyle)
	{
		if(t != nullptr && t->type == typeid(wxButton*).hash_code())
		{
			long flags;
			prop->GetValue().Convert(&flags);
			wxButton* old = dynamic_cast<wxButton*>(m_SelectedWidget);
			decltype(old) nw = GuiEditor::Get()->AddWidget<std::decay<decltype(*old)>::type>(panel, wxID_ANY, old->GetLabelText(), old->GetPosition(), old->GetSize(), flags);
			old->Destroy();
			auto nodeHandler = widgets.extract(old);
			nodeHandler.key() = nw;
			widgets.insert(std::move(nodeHandler));
			old = nullptr;
			m_SelectedWidget = nw;
		}
	}
}

void GuiEditor::AddFlagsToPropgrid()
{
	wxPGChoices m_combinedFlags;
	m_combinedFlags.Add(WXSIZEOF(button_style_flags), button_style_flags, button_style_values);
	panel->m_pgButtonStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("Button flags"), wxPG_LABEL, m_combinedFlags));
	panel->m_propertyGrid->SetPropertyAttribute("Button flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags2;
	m_combinedFlags2.Add(WXSIZEOF(slider_style_flags), slider_style_flags, slider_style_values);
	panel->m_pgSliderStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("Slider flags"), wxPG_LABEL, m_combinedFlags2));
	panel->m_propertyGrid->SetPropertyAttribute("Slider flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags3;
	m_combinedFlags3.Add(WXSIZEOF(statictext_style_flags), statictext_style_flags, statictext_style_values);
	panel->m_pgStaticTextStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("StaticText flags"), wxPG_LABEL, m_combinedFlags3));
	panel->m_propertyGrid->SetPropertyAttribute("StaticText flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags4;
	m_combinedFlags4.Add(WXSIZEOF(textctrl_style_flags), textctrl_style_flags, textctrl_style_values);
	panel->m_pgTextCtrlStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("TextControl flags"), wxPG_LABEL, m_combinedFlags4));
	panel->m_propertyGrid->SetPropertyAttribute("TextControl flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags5;
	m_combinedFlags5.Add(WXSIZEOF(combobox_style_flags), combobox_style_flags, combobox_style_values);
	panel->m_pgComboBoxStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("ComboBox flags"), wxPG_LABEL, m_combinedFlags5));
	panel->m_propertyGrid->SetPropertyAttribute("ComboBox flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags6;
	m_combinedFlags6.Add(WXSIZEOF(choice_style_flags), choice_style_flags, choice_style_values);
	panel->m_pgChoiseStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("Choice flags"), wxPG_LABEL, m_combinedFlags6));
	panel->m_propertyGrid->SetPropertyAttribute("Choice flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags7;
	m_combinedFlags7.Add(WXSIZEOF(checkbox_style_flags), checkbox_style_flags, checkbox_style_values);
	panel->m_pgCheckboxStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("Checkbox flags"), wxPG_LABEL, m_combinedFlags7));
	panel->m_propertyGrid->SetPropertyAttribute("Checkbox flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);

	wxPGChoices m_combinedFlags8;
	m_combinedFlags8.Add(WXSIZEOF(gauge_style_flags), gauge_style_flags, gauge_style_values);
	panel->m_pgGaugeStyle = panel->m_propertyGrid->Append(new wxFlagsProperty(wxT("Gauge flags"), wxPG_LABEL, m_combinedFlags8));
	panel->m_propertyGrid->SetPropertyAttribute("Gauge flags", wxPG_BOOL_USE_CHECKBOX, true, wxPG_RECURSE);
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
		wxPoint pos = dynamic_cast<wxWindow*>(m_SelectedWidget)->GetPosition();
		wxSize size = dynamic_cast<wxWindow*>(m_SelectedWidget)->GetSize();
		dc.Clear();
		dc.SetPen(wxPen(*wxRED, 1));
		dc.DrawRectangle(pos.x - 1, pos.y - 1, size.x + 2, size.y + 2);
	}
}

void GuiEditor::OnOpen()
{
	wxFileDialog openFileDialog(panel, _("Open XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if(openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	OnDestroyAll();
	file_path = openFileDialog.GetPath();
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
		wxWindow* w = dynamic_cast<wxStaticText*>(x.first);
		w->Destroy();
		delete x.second;
	}
	widgets.clear();
}

void GuiEditor::DuplicateWidget()
{
	auto x = widgets.find(m_SelectedWidget);
	if(x != widgets.end())
	{
		if(x->second->type == typeid(wxButton*).hash_code())  /* I know this is against polymorhism in C++, but no time for better one... */
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

void GuiEditor::AdjustWidgetPos(std::pair<wxWindow*, Widget*> item, int8_t x, int8_t y)
{
	auto [window, widget] = item;
	wxPoint pos = window->GetPosition();
	if(x != std::numeric_limits<int8_t>::max())
	{
		if(x > 0)
			pos.x += m_speed;
		else
			pos.x -= m_speed;
	}
	if(y != std::numeric_limits<int8_t>::max())
	{
		if(y > 0)
			pos.y += m_speed;
		else
			pos.y -= m_speed;
	}
	
	window->SetPosition(pos);
	panel->UpdatePropgrid({ widget->id, &widget->name, window });
}

void GuiEditor::AdjustWidgetSize(std::pair<wxWindow*, Widget*> item, int8_t x, int8_t y)
{
	auto [window, widget] = item;
	wxSize pos = window->GetSize();
	if(x != std::numeric_limits<int8_t>::max())
	{
		if(x > 0)
			pos.x += m_speed;
		else
			pos.x -= m_speed;
	}
	if(y != std::numeric_limits<int8_t>::max())
	{
		if(y > 0)
			pos.y += m_speed;
		else
			pos.y -= m_speed;
	}
	window->SetSize(pos);
	panel->UpdatePropgrid({ widget->id, &widget->name, window });
}

void ParseDefaultWidgetFormat(const boost::property_tree::ptree::value_type& v, wxPoint& pos, wxSize& size, wxSize& min_size, wxSize& max_size,
	int& fontsize, int& fontfamily, int& fontstyle, int& fontweight, int& is_underlined, char* fontname, uint8_t fg_colors[3], uint8_t bg_colors[3])
{
	bool err = 0;
	if(sscanf(v.second.get<std::string>("<xmlattr>.pos").c_str(), "%d,%d", &pos.x, &pos.y) != 2)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.size").c_str(), "%d,%d", &size.x, &size.y) != 2)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.min_size").c_str(), "%d,%d", &min_size.x, &min_size.y) != 2)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.max_size").c_str(), "%d,%d", &max_size.x, &max_size.y) != 2)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.fg_color").c_str(), "%hhd,%hhd,%hhd", &fg_colors[0], &fg_colors[1], &fg_colors[2]) != 3)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.bg_color").c_str(), "%hhd,%hhd,%hhd", &bg_colors[0], &bg_colors[1], &bg_colors[2]) != 3)
		err = 1;
	if(sscanf(v.second.get<std::string>("<xmlattr>.font").c_str(), "%d,%d,%d,%d,%d,%[^\n]s", &fontsize, &fontfamily, &fontstyle, &fontweight, &is_underlined, fontname) != 6)
		err = 1;

	if(err)
		throw std::invalid_argument("Error while parsing XML file");
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
	const std::string filename = file_path.ToStdString();
	boost::property_tree::ptree pt;
	read_xml(filename, pt);

	for(const boost::property_tree::ptree::value_type& v : pt.get_child("wxCreatorXmlFile"))
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
			wxButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "combobox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxArrayString a;
			wxComboBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, a, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "choise")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxArrayString a;
			wxChoice* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, a, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "listbox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxListBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "checkbox")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxCheckBox* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "radiobutton")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxRadioButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "staticline")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxStaticLine* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "slider")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSlider* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.value")), utils::stoi<int>(v.second.get<std::string>("<xmlattr>.min")),
				utils::stoi<int>(v.second.get<std::string>("<xmlattr>.max")), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "gauge")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxGauge* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.max")), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "spincontrol")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSpinCtrl* tmp = AddWidget<wxSpinCtrl>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")),
				utils::stoi<int>(v.second.get<std::string>("<xmlattr>.min")), utils::stoi<int>(v.second.get<std::string>("<xmlattr>.max")), utils::stoi<int>(v.second.get<std::string>("<xmlattr>.value")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "spincontroldouble")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxSpinCtrlDouble* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")),
				utils::stoi<int>(v.second.get<std::string>("<xmlattr>.min")), utils::stoi<int>(v.second.get<std::string>("<xmlattr>.max")), utils::stoi<int>(v.second.get<std::string>("<xmlattr>.value")), 
				utils::stoi<int>(v.second.get<std::string>("<xmlattr>.inc")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "textcontrol")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxTextCtrl* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "togglebutton")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxToggleButton* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
			SetDefaultWidgetFormat(tmp, v.second.get<std::string>("<xmlattr>.tooltip"), pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);
		}
		else if(type == "statictext")
		{
			uint8_t fg_colors[3], bg_colors[3];
			ParseDefaultWidgetFormat(v, pos, size, min_size, max_size, fontsize, fontfamily, fontstyle, fontweight, is_underlined, fontname, fg_colors, bg_colors);			
			wxStaticText* tmp = AddWidget<std::decay<decltype(*tmp)>::type>(panel, wxID_ANY, v.second.get<std::string>("<xmlattr>.label"), pos, size, utils::stoi<int>(v.second.get<std::string>("<xmlattr>.flags")));
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
		if(x.second->type == typeid(wxButton*).hash_code())  /* I know this is against polymorhism in C++, but no time for better one... */
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
	if((min_size.x != -1 || min_size.y != -1) && t->GetSize() != min_size)
		wxstr += wxString::Format("%s->SetMinSize(wxPoint(%d, %d));\n", obj->name, min_size.x, min_size.y);
	wxSize max_size = t->GetMaxSize();
	if(max_size.x != -1 || max_size.y != -1)
		wxstr += wxString::Format("%s->SetMaxSize(wxPoint(%d, %d));\n", obj->name, max_size.x, max_size.y);
	wxFont font = t->GetFont();
	if(obj->font_changed)
	{
		wxString strFontPointSize;
		if(font.GetPointSize() != -1)
			strFontPointSize = wxString::Format("%d", font.GetPointSize());
		else
			strFontPointSize = wxString("wxNORMAL_FONT->GetPointSize()");

		std::string& fontfamily = fontfamily_map[wxFONTFAMILY_UNKNOWN];
		if(fontfamily_map.find(font.GetFamily()) != fontfamily_map.end())
			fontfamily = fontfamily_map[font.GetFamily()];
		std::string& fontstyle = fontstyle_map[wxFONTSTYLE_MAX];
		if(fontstyle_map.find(font.GetStyle()) != fontstyle_map.end())
			fontstyle = fontstyle_map[font.GetStyle()];
		std::string& fontweight = fontweight_map[wxFONTSTYLE_MAX];
		if(fontweight_map.find(font.GetWeight()) != fontweight_map.end())
			fontweight = fontweight_map[font.GetWeight()];

		wxstr += wxString::Format("%s->SetFont( wxFont(%s, %s, %s, %s, %s, wxT(\"%s\")));\n",
			obj->name, strFontPointSize, fontfamily, fontstyle, fontweight, font.GetUnderlined() ? wxString("true") : wxString("false"), font.GetFaceName());
		DBG("font\n");
	}
	wxstr += "\n";
}

void GuiEditor::GenerateCode(wxString& str)
{
#define FLAG(val) val##_style_values, val##_style_flags, WXSIZEOF(val##_style_flags)
	for(auto& x : widgets)
	{
		if(x.second->type == typeid(wxButton*).hash_code())
		{
			wxButton* t = dynamic_cast<wxButton*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(button));
			str += wxString::Format("%s = new wxButton(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
		}
		else if(x.second->type == typeid(wxComboBox*).hash_code())
		{
			wxComboBox* t = dynamic_cast<wxComboBox*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(combobox));
			str += wxString::Format("%s = new wxComboBox(this, wxID_ANY, wxT(\"%s\"), wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
			AddFontAndColor(str, x.first, x.second);
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
		else if(x.second->type == typeid(wxStaticText*).hash_code())
		{
			wxStaticText* t = dynamic_cast<wxStaticText*>(x.first);
			wxString flagsStr;
			AddFlags(flagsStr, x.first, x.second, FLAG(statictext));
			str += wxString::Format("%s = new wxStaticText(this, wxID_ANY, %s, wxPoint(%d, %d), wxSize(%d, %d), %s);\n",
				x.second->name, t->GetLabelText(), t->GetPosition().x, t->GetPosition().y, t->GetSize().x, t->GetSize().y, flagsStr);
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
