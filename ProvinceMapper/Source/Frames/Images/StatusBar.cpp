#include "StatusBar.h"

#include "ImageCanvas.h"
#include "Log.h"

StatusBar::StatusBar(wxWindow* parent):
	 wxFrame(parent, wxID_ANY, "Image Toolbar", wxDefaultPosition, wxSize(350, 100), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL), eventHandler(parent)
{
	auto* holderPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxEXPAND);
	holderPanel->SetBackgroundColour(wxColour(230, 230, 230));
	auto* sizer = new wxFlexGridSizer(2, 2, 5);
	holderPanel->SetSizer(sizer);

	auto* sZoomText = new wxStaticText(holderPanel, wxID_ANY, "Source Zoom:", wxDefaultPosition, wxDefaultSize);
	auto* tZoomText = new wxStaticText(holderPanel, wxID_ANY, "Target Zoom:", wxDefaultPosition, wxDefaultSize);

	sourceZoomField = new wxTextCtrl(holderPanel, 0, "100", wxDefaultPosition, wxSize(50, 20), wxBORDER_DEFAULT | wxTE_PROCESS_ENTER);
	sourceZoomField->Bind(wxEVT_TEXT_ENTER, &StatusBar::onZoomChanged, this);

	targetZoomField = new wxTextCtrl(holderPanel, 1, "100", wxDefaultPosition, wxSize(50, 20), wxBORDER_DEFAULT | wxTE_PROCESS_ENTER);
	targetZoomField->Bind(wxEVT_TEXT_ENTER, &StatusBar::onZoomChanged, this);

	sizer->Add(sZoomText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5));
	sizer->Add(sourceZoomField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5));

	sizer->Add(tZoomText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5));
	sizer->Add(targetZoomField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5));

	auto* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(holderPanel, wxSizerFlags(1).Expand());
	SetSizer(boxSizer);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void StatusBar::onZoomChanged(wxCommandEvent& evt)
{
	if (evt.GetId() == 0)
	{
		const auto input = sourceZoomField->GetValue();
		int zoomLevel;
		try
		{
			zoomLevel = std::stoi(sourceZoomField->GetValue().ToStdString());
		}
		catch (std::exception&)
		{
			zoomLevel = 100;
		}
		if (zoomLevel > 1000)
		{
			zoomLevel = 1000;
		}
		else if (zoomLevel < 10)
		{
			zoomLevel = 10;
		}
		sourceZoomField->ChangeValue(std::to_string(zoomLevel));
		wxCommandEvent event(wxEVT_REFRESH);
		event.SetId(0);
		event.SetInt(zoomLevel);
		eventHandler->QueueEvent(event.Clone());
	}
	else if (evt.GetId() == 1)
	{
		const auto input = targetZoomField->GetValue();
		int zoomLevel;
		try
		{
			zoomLevel = std::stoi(targetZoomField->GetValue().ToStdString());
		}
		catch (std::exception&)
		{
			zoomLevel = 100;
		}
		if (zoomLevel > 1000)
		{
			zoomLevel = 1000;
		}
		else if (zoomLevel < 10)
		{
			zoomLevel = 10;
		}
		wxCommandEvent event(wxEVT_REFRESH);
		event.SetId(1);
		event.SetInt(zoomLevel);
		eventHandler->QueueEvent(event.Clone());
	}
	Refresh();
}

void StatusBar::setSourceZoom(const int zoomLevel) const
{
	sourceZoomField->ChangeValue(std::to_string(zoomLevel));
}

void StatusBar::setTargetZoom(const int zoomLevel) const
{
	targetZoomField->ChangeValue(std::to_string(zoomLevel));
}
