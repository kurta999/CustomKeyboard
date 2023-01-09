#include "pch.hpp"

wxBEGIN_EVENT_TABLE(MapConverterPanel, wxPanel)
EVT_FILEPICKER_CHANGED(ID_FilePickerMapConverter, MapConverterPanel::OnFileSelected)
EVT_SIZE(MapConverterPanel::OnSize)
wxEND_EVENT_TABLE()

MapConverterPanel::MapConverterPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Select a file, paste it's content or Drag'n'Drop to textbox below\nWhen done, click on Generate!"), wxDefaultPosition, wxDefaultSize, 0));
	m_FilePicker = new wxFilePickerCtrl(this, ID_FilePickerMapConverter, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxSize(500, 25), wxFLP_DEFAULT_STYLE);
	bSizer1->Add(m_FilePicker);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Offset X Y Z (Example: 45 -1000 0)"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
	m_Offset = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, 25), wxTE_MULTILINE);
	bSizer1->Add(m_Offset);

	wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_OnlyCreateVehicle = new wxCheckBox(this, wxID_ANY, "Only CreateVehicle?", wxDefaultPosition, wxDefaultSize, 0);
	h_sizer->Add(m_OnlyCreateVehicle);
	m_VehCompInline = new wxCheckBox(this, wxID_ANY, "Veh components inline?", wxDefaultPosition, wxDefaultSize, 0);
	h_sizer->Add(m_VehCompInline);
	m_SaveNumberPlate = new wxCheckBox(this, wxID_ANY, "Convert numberplate?", wxDefaultPosition, wxDefaultSize, 0);
	h_sizer->Add(m_SaveNumberPlate);
	m_HideWhenAlphaIsSet = new wxCheckBox(this, wxID_ANY, "Hide when alpha is set?", wxDefaultPosition, wxDefaultSize, 0);
	h_sizer->Add(m_HideWhenAlphaIsSet);
	m_ConvertItemName = new wxCheckBox(this, wxID_ANY, "Convert item name?", wxDefaultPosition, wxDefaultSize, 0);
	h_sizer->Add(m_ConvertItemName);

	bSizer1->Add(h_sizer);

	m_Input = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(310, 310), wxTE_MULTILINE);
#ifdef _WIN32
	m_Input->DragAcceptFiles(true);  /* This one doesn't work with GTK for some reason... */
#endif
	m_Input->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(MapConverterPanel::OnFileDrop), NULL, this);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	bSizer2->Add(m_Input, wxSizerFlags(1).Top().Expand());

	m_Output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(310, 310), wxTE_MULTILINE);
	bSizer2->Add(m_Output, wxSizerFlags(1).Expand());
	bSizer1->Add(bSizer2, wxSizerFlags(1).Expand());

	wxBoxSizer* h_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_OkButton = new wxButton(this, wxID_ANY, wxT("Convert"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_OkButton);
	h_sizer_2->AddSpacer(100);
	
	m_ClearButton = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	h_sizer_2->Add(m_ClearButton);
	m_ClearButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			m_Input->Clear();
			m_Output->Clear();
			m_FilePicker->SetFileName(wxFileName());
			m_OkButton->SetForegroundColour(*wxBLACK);
			path.Clear();
		});

	bSizer1->Add(h_sizer_2);

	this->SetSizerAndFit(bSizer1);
	this->Layout();

	m_OkButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			wxString str = m_Input->GetValue();
			Vec3D offset;
			MapConverterFlags flags = MapConverterFlags::NONE;
			
			sscanf(m_Offset->GetValue().ToStdString().c_str(), "%f%*c%f%*c%f%*c", &offset.x, &offset.y, &offset.z);

			if(m_OnlyCreateVehicle->IsChecked())
				flags |= MapConverterFlags::ONLY_CREATE_VEHICLE;
			if(m_VehCompInline->IsChecked())
				flags |= MapConverterFlags::VEHICLE_COMPONENTS_INLINE;
			if(m_SaveNumberPlate->IsChecked())
				flags |= MapConverterFlags::SAVE_NUMBER_PLATE;
			if(m_HideWhenAlphaIsSet->IsChecked())
				flags |= MapConverterFlags::HIDE_WHEN_ALPHA_NOT_255;
			if(m_ConvertItemName->IsChecked())
				flags |= MapConverterFlags::CONVERT_ELEMENT_NAME;

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

			std::string output;
			try
			{
				output = MapConverter::Get()->MtaToSamp(input, flags, offset);
			}
			catch(std::exception& e)
			{
				LOG(LogLevel::Error, "Exception: {}", e.what());
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}

			if(!output.empty())
			{
#ifdef _WIN32
				boost::algorithm::replace_all(output, "\n", "\r\n");  /* LF isn't enough for TextCtrl for some reason... */
#endif
				m_Output->Clear();
				wxString wxout(output);
				m_Output->SetValue(wxout);
				 
				LOG(LogLevel::Verbose, "wxOut: {}", wxout.ToStdString().substr(0, 128));
				m_OkButton->SetForegroundColour(*wxBLACK);
			}
			else
			{
				LOG(LogLevel::Verbose, "Empty output");
			}
			path.Clear();
		});
}

void MapConverterPanel::OnFileDrop(wxDropFilesEvent& event)
{
	if(event.GetNumberOfFiles() > 0)
	{
		wxString* dropped = event.GetFiles();
		path = *dropped;
		m_FilePicker->SetFileName(wxFileName(path));

		HandleInputFileSelect(path);
	}
}

void MapConverterPanel::OnFileSelected(wxFileDirPickerEvent& event)
{
	path = event.GetPath();
	HandleInputFileSelect(path);
}

void MapConverterPanel::OnSize(wxSizeEvent& event)
{
	wxSize a = event.GetSize();
	event.Skip(true);
}

void MapConverterPanel::HandleInputFileSelect(wxString& path)
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
			m_OkButton->SetForegroundColour(*wxRED);
			m_Input->SetValue(input);
		}
	}

}