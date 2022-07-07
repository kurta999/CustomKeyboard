#pragma once

#include "utils/CSingleton.hpp"
#include <wx/wx.h>

#include <inttypes.h>
#include <wx/treectrl.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>

#include <map>

#include "gui/MainFrame.hpp"
#include "CustomKeyboard.hpp"

enum class wxTypes : int
{
	Button,
	ComboBox,
	Choise,
	ListBox,
	CheckBox,
	RadioButton,
	StaticLine,
	Slider,
	Gauge,
	Text,
	SpinControl,
	SpinCtrlDouble,
	TextControl,
	ToggleButton,
	SearchCtrl,
	ColorPicker,
	FontPicker,
	FilePicker,
	DirPicker,
	DatePicker,
	TimePicker,
	CalendarCtrl,
	GenericDirCtrl,
	SpinButton,
	Maximum,
	Invalid = 0xFF,
};

class Widget
{
public:
	Widget(size_t _type, std::string typeid_name) :
		type(_type)
	{
		id = item_ids[type];
		++item_ids[type];

		size_t pos = typeid_name.find("wx", 0);  /* typeid(T).name() returns: "class wxButton * __ptr64" */
		size_t pointer = typeid_name.find("*", 0);
		std::string widget_type = typeid_name.substr(pos, pointer - pos - 1);

		name = wxString::Format("%s_%d", widget_type, id);
		fg_color_changed = bg_color_changed = font_changed = 0;
	}
	~Widget()
	{
		if(item_ids[type] > 0)
			--item_ids[type];
	}

	size_t type;
	uint16_t id;
	wxString name;
	uint8_t fg_color_changed;
	uint8_t bg_color_changed;
	uint8_t font_changed;
	wxTreeItemId item_id;

	static inline std::map<size_t, uint16_t> item_ids;
};

class GuiEditor : public CSingleton < GuiEditor >
{
	friend class CSingleton < GuiEditor >;

	void SaveWidgets();
	void LoadWidgets();
	void AddFlags(wxString& wxstr, void* widget, Widget* obj, const long* to_pointer, const wxString* to_pointer_str, const long max_array_size);
	void AddFontAndColor(wxString& wxstr, void* widget, Widget* obj);
	void DuplicateWidget();
	void AdjustWidgetPos(std::pair<wxWindow*, Widget*> item, int8_t x, int8_t y);
	void AdjustWidgetSize(std::pair<wxWindow*, Widget*> item, int8_t x, int8_t y);
public:
	void Init(GuiEditorMain* panel_);
	void OnClick(void* object);
	void OnMouseMotion(wxPoint& pos);
	void OnKeyDown(int keycode);
	void OnPropertyGridChange(wxPGProperty* prop);

	void OnPaint();
	void OnOpen();
	void OnSave();
	void OnSaveAs();
	void OnDestroyAll();
	void GenerateCode(wxString& str);

	template <typename T, typename... Args> T* AddWidget(Args &&...args)
	{
		T* o = new T(std::forward<Args>(args)...);
		widgets[dynamic_cast<wxObject*>(o)] = new Widget(typeid(o).hash_code(), typeid(o).name());
		o->Bind(wxEVT_LEFT_DOWN, &GuiEditorMain::OnClick, panel);
		return o;
	}

	Widget* FindwxText(wxObject* object_to_find = nullptr);
	void MarkSelectedItem();

private:
	void AddFlagsToPropgrid();

	GuiEditorMain* panel = nullptr;
	std::unordered_map<wxObject*, Widget*> widgets;
	wxObject* m_SelectedWidget = nullptr;
	wxString file_path;
	int m_speed = 1;

	std::map<int, std::string> fontfamily_map =
	{
		{wxFONTFAMILY_DEFAULT, "wxFONTFAMILY_DEFAULT"},
		{wxFONTFAMILY_DECORATIVE, "wxFONTFAMILY_DECORATIVE"},
		{wxFONTFAMILY_ROMAN, "wxFONTFAMILY_ROMAN"},
		{wxFONTFAMILY_SCRIPT, "wxFONTFAMILY_SCRIPT"},
		{wxFONTFAMILY_SWISS, "wxFONTFAMILY_SWISS"},
		{wxFONTFAMILY_MODERN, "wxFONTFAMILY_MODERN"},
		{wxFONTFAMILY_MODERN, "wxFONTFAMILY_MODERN"},
		{wxFONTFAMILY_TELETYPE, "wxFONTFAMILY_TELETYPE"},
		{wxFONTFAMILY_MAX, "wxFONTFAMILY_MAX"},
		{wxFONTFAMILY_UNKNOWN, "wxFONTFAMILY_UNKNOWN"},
	};

	std::map<int, std::string> fontstyle_map =
	{
		{wxFONTSTYLE_NORMAL, "wxFONTSTYLE_NORMAL"},
		{wxFONTSTYLE_ITALIC, "wxFONTSTYLE_ITALIC"},
		{wxFONTSTYLE_SLANT, "wxFONTSTYLE_SLANT"},
		{wxFONTSTYLE_MAX, "wxFONTSTYLE_MAX"},
	};

	std::map<int, std::string> fontweight_map =
	{
		{wxFONTWEIGHT_INVALID, "wxFONTWEIGHT_INVALID"},
		{wxFONTWEIGHT_THIN, "wxFONTWEIGHT_THIN"},
		{wxFONTWEIGHT_EXTRALIGHT, "wxFONTWEIGHT_EXTRALIGHT"},
		{wxFONTWEIGHT_LIGHT, "wxFONTWEIGHT_LIGHT"},
		{wxFONTWEIGHT_NORMAL, "wxFONTWEIGHT_NORMAL"},
		{wxFONTWEIGHT_MEDIUM, "wxFONTWEIGHT_MEDIUM"},
		{wxFONTWEIGHT_SEMIBOLD, "wxFONTWEIGHT_SEMIBOLD"},
		{wxFONTWEIGHT_BOLD, "wxFONTWEIGHT_BOLD"},
		{wxFONTWEIGHT_EXTRABOLD, "wxFONTWEIGHT_EXTRABOLD"},
		{wxFONTWEIGHT_HEAVY, "wxFONTWEIGHT_HEAVY"},
		{wxFONTWEIGHT_EXTRAHEAVY, "wxFONTWEIGHT_EXTRAHEAVY"},
		{wxFONTWEIGHT_MAX, "wxFONTWEIGHT_MAX"},
	};
};