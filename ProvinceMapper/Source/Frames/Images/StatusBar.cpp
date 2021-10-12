#include "StatusBar.h"
#include "Configuration/Configuration.h"
#include "ImageCanvas.h"
#include "Log.h"

wxDEFINE_EVENT(wxEVT_TOGGLE_TRIANGULATE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_LOCK, wxCommandEvent);

StatusBar::StatusBar(wxWindow* parent, const wxPoint& position, std::shared_ptr<Configuration> theConfiguration):
	 wxFrame(parent, wxID_ANY, "Image Toolbar", position, wxSize(500, 100), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL | wxSTAY_ON_TOP),
	 configuration(std::move(theConfiguration)),
	 eventHandler(parent)
{
	Bind(wxEVT_CLOSE_WINDOW, &StatusBar::onClose, this);
	Bind(wxEVT_MOVE, &StatusBar::onMove, this);

	auto* holderPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxEXPAND);
	holderPanel->SetBackgroundColour(wxColour(230, 230, 230));
	auto* sizer = new wxFlexGridSizer(8, 5, 5);
	holderPanel->SetSizer(sizer);

	lockButton = new wxButton(holderPanel, wxID_ANY, "Lock", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Lock");
	lockButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &StatusBar::onLock, this);
	lockStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(45, 15));
	lockStatus->SetBackgroundColour("gray");

	auto* sZoomText = new wxStaticText(holderPanel, wxID_ANY, "Source Zoom:", wxDefaultPosition, wxDefaultSize);
	auto* tZoomText = new wxStaticText(holderPanel, wxID_ANY, "Target Zoom:", wxDefaultPosition, wxDefaultSize);

	sourceZoomField = new wxTextCtrl(holderPanel, 0, "100", wxDefaultPosition, wxSize(50, 20), wxBORDER_DEFAULT | wxTE_PROCESS_ENTER);
	sourceZoomField->Bind(wxEVT_TEXT_ENTER, &StatusBar::onZoomChanged, this);
	targetZoomField = new wxTextCtrl(holderPanel, 1, "100", wxDefaultPosition, wxSize(50, 20), wxBORDER_DEFAULT | wxTE_PROCESS_ENTER);
	targetZoomField->Bind(wxEVT_TEXT_ENTER, &StatusBar::onZoomChanged, this);

	auto* sourceResetButton = new wxButton(holderPanel, 0, "Reset", wxDefaultPosition, wxDefaultSize);
	sourceResetButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &StatusBar::onZoomResetButton, this);
	auto* targetResetButton = new wxButton(holderPanel, 1, "Reset", wxDefaultPosition, wxDefaultSize);
	targetResetButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &StatusBar::onZoomResetButton, this);

	triangulateText = new wxStaticText(holderPanel, wxID_ANY, "Triangulate:", wxDefaultPosition, wxDefaultSize);
	triangulateButton = new wxButton(holderPanel, wxID_ANY, "Enable", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Toggle Triangulation");
	triangulateButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &StatusBar::onTriangulate, this);

	sTriangulate1 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	sTriangulate1->SetBackgroundColour("gray");
	sTriangulate2 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	sTriangulate2->SetBackgroundColour("gray");
	sTriangulate3 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	sTriangulate3->SetBackgroundColour("gray");
	tTriangulate1 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	tTriangulate1->SetBackgroundColour("gray");
	tTriangulate2 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	tTriangulate2->SetBackgroundColour("gray");
	tTriangulate3 = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	tTriangulate3->SetBackgroundColour("gray");

	sizer->Add(lockButton, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sZoomText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sourceZoomField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sourceResetButton, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(triangulateText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sTriangulate1, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sTriangulate2, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sTriangulate3, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxTOP, 5).Center());

	sizer->Add(lockStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(tZoomText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(targetZoomField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(targetResetButton, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(triangulateButton, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(tTriangulate1, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(tTriangulate2, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());
	sizer->Add(tTriangulate3, wxSizerFlags(0).Align(wxVERTICAL).Border(wxRIGHT | wxBOTTOM, 5).Center());

	auto* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(holderPanel, wxSizerFlags(1).Expand());
	SetSizer(boxSizer);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void StatusBar::onZoomChanged(const wxCommandEvent& evt)
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
		targetZoomField->ChangeValue(std::to_string(zoomLevel));
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

void StatusBar::onZoomResetButton(const wxCommandEvent& evt)
{
	constexpr auto zoomLevel = 100;
	wxCommandEvent event(wxEVT_REFRESH);
	if (evt.GetId() == 0)
	{
		sourceZoomField->ChangeValue(std::to_string(zoomLevel));
		event.SetId(0);
	}
	else if (evt.GetId() == 1)
	{
		targetZoomField->ChangeValue(std::to_string(zoomLevel));
		event.SetId(1);
	}
	event.SetInt(zoomLevel);
	eventHandler->QueueEvent(event.Clone());
	Refresh();
}

void StatusBar::onTriangulate(wxCommandEvent& evt)
{
	if (triangulate)
	{
		triangulate = false;
		sTriangulate1->SetBackgroundColour("gray");
		sTriangulate2->SetBackgroundColour("gray");
		sTriangulate3->SetBackgroundColour("gray");
		tTriangulate1->SetBackgroundColour("gray");
		tTriangulate2->SetBackgroundColour("gray");
		tTriangulate3->SetBackgroundColour("gray");
		triangulateButton->SetLabelText("Enable");
		triangulateText->SetBackgroundColour(wxColor(230, 230, 230));
	}
	else
	{
		triangulate = true;
		sTriangulate1->SetBackgroundColour("red");
		sTriangulate2->SetBackgroundColour("red");
		sTriangulate3->SetBackgroundColour("red");
		tTriangulate1->SetBackgroundColour("red");
		tTriangulate2->SetBackgroundColour("red");
		tTriangulate3->SetBackgroundColour("red");
		triangulateButton->SetLabelText("Disable");
		triangulateText->SetBackgroundColour("red");
	}
	const wxCommandEvent event(wxEVT_TOGGLE_TRIANGULATE);
	eventHandler->QueueEvent(event.Clone());
	Refresh();
}

void StatusBar::onClose(wxCloseEvent& event)
{
	configuration->setStatusBarOn(false);
	configuration->save();
	Hide();
}

void StatusBar::setPointPlaced(const int pointID)
{
	switch (pointID)
	{
		case 1:
			sTriangulate1->SetBackgroundColour("green");
			break;
		case 2:
			sTriangulate2->SetBackgroundColour("green");
			break;
		case 3:
			sTriangulate3->SetBackgroundColour("green");
			break;
		case 4:
			tTriangulate1->SetBackgroundColour("green");
			break;
		case 5:
			tTriangulate2->SetBackgroundColour("green");
			break;
		case 6:
			tTriangulate3->SetBackgroundColour("green");
			break;
		default:
			break;
	}
	Refresh();
}

void StatusBar::setTriangulationSane(const bool sane)
{
	if (triangulate)
	{
		if (sane)
			triangulateText->SetBackgroundColour("green");
		else
			triangulateText->SetBackgroundColour("red");
	}
	else
	{
		triangulateText->SetBackgroundColour(wxColor(230, 230, 230));
	}
	Refresh();
}

void StatusBar::onMove(wxMoveEvent& event)
{
	const auto position = GetPosition();
	configuration->setStatusBarPos(position.x, position.y);
	configuration->save();
	event.Skip();
}

void StatusBar::onLock(wxCommandEvent& evt)
{
	auto event = wxCommandEvent(wxEVT_LOCK);
	if (lock)
	{
		lock = false;
		lockStatus->SetBackgroundColour("gray");
		lockButton->SetLabel("Lock");
		event.SetInt(0);
	}
	else
	{
		lock = true;
		lockStatus->SetBackgroundColour("green");
		lockButton->SetLabel("Unlock");
		event.SetInt(1);
	}
	Refresh();
	eventHandler->AddPendingEvent(event);
}
