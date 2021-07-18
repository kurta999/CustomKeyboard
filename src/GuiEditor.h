#pragma once

#include "utils/CSingleton.h"
#include <wx/wx.h>

#include <inttypes.h>
#include <wx/treectrl.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>

#include <map>

#include "gui/main_frame.h"
#include "CustomKeyboard.h"

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
		fg_color_changed = bg_color_changed = 0;
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
	wxTreeItemId item_id;

	static std::map<size_t, uint16_t> item_ids;
};

class GuiEditor : public CSingleton < GuiEditor >
{
	friend class CSingleton < GuiEditor >;

	void SaveWidgets();
	void LoadWidgets();
	void AddFlags(wxString& wxstr, void* widget, Widget* obj, const long* to_pointer, const wxString* to_pointer_str, const long max_array_size);
	void AddFontAndColor(wxString& wxstr, void* widget, Widget* obj);
	void DuplicateWidget();

public:
	void Init(GuiEditorMain* panel_);
	void OnClick(void* object);
	void OnMouseMotion(wxPoint& pos);
	void OnKeyDown(int keycode);
	
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
	GuiEditorMain* panel;
	std::unordered_map<wxObject*, Widget*> widgets;
	wxObject* m_SelectedWidget = nullptr;
	wxString file_path;
	int m_speed = 1;
};