#include "MainFrame.h"
#include "wx/splitter.h"
#include "PointData.h"
#include "PointWindow.h"

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size): wxFrame(nullptr, wxID_ANY, title, pos, size)
{
	// MainFrame is what holds the entire application together, visually and logically.
	// It receives events from both the visual side (imageTabs and pointWindow), and the logical/file side (pointMapper),
	// and dispatches in response.

	Bind(wxEVT_CHANGE_TAB, &MainFrame::onChangeTab, this);
	Bind(wxEVT_UPDATE_POINT, &MainFrame::onUpdatePoint, this);
	Bind(wxEVT_POINT_PLACED, &MainFrame::onPointPlaced, this);
	Bind(wxEVT_DESELECT_POINT, &MainFrame::onDeselectWorkingPoint, this);
	Bind(wxEVT_DELETE_WORKING_POINT, &MainFrame::onDeleteWorkingPoint, this);
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
	Bind(wxEVT_MENU, &MainFrame::onExportPoints, this, wxID_FILE1);
	Bind(wxEVT_MENU, &MainFrame::onExportAdjustedMap, this, wxID_FILE2);
}

void MainFrame::initFrame()
{
	vbox = new wxFlexGridSizer(1, 2, 0, 0); // This is a "Sizer". It visually defines our application as a grid, of 1 row and 2 columns.
	notebook = new wxNotebook(this, wxID_ANY); // This is a "Notebook" for the tab layout of the ImageTabs.

	imageTabFrom = new ImageTab(notebook, ImageTabSelector::SOURCE); // We create the first image here
	notebook->AddPage(imageTabFrom, "Source"); // And give it to the notebook
	imageTabTo = new ImageTab(notebook, ImageTabSelector::TARGET);
	notebook->AddPage(imageTabTo, "Target");

	pointWindow = new PointWindow(this); // Pointwindow goes to the right.
	pointWindow->SetMinSize(wxSize(200, 1000));

	vbox->Add(pointWindow, wxSizerFlags(0).Expand()); // And finally we register the graphical components with the sizer so they get positioned.
	vbox->Add(notebook, wxSizerFlags(1).Expand());

	notebook->SetMinSize(wxSize(9000, 5000)); // Since out imageTabs do not contain actual text, but dynamically rendered images on a Device Context, we don't know their size.

	SetSizer(vbox); // We apply the sizer to mainframe

	// Tell the pictures about preloaded points
	for (const auto& coPoint: *pointMapper.getCoPoints())
	{
		if (coPoint->getSource())
			imageTabFrom->registerPoint(*coPoint->getSource());
		if (coPoint->getTarget())
			imageTabTo->registerPoint(*coPoint->getTarget());
	}
	// store the reference to the entire thing in pointWindow.
	pointWindow->loadCoPoints(pointMapper.getCoPoints());

	// and have them displayed.
	imageTabFrom->refresh();
	imageTabTo->refresh();
	pointWindow->redrawGrid();
}

void MainFrame::onPointPlaced(wxCommandEvent& event)
{
	// We just got an event from one of the ImageTabs that a new point is registered.
	// This may be a completely new point, or half of an existing pair, but that's
	// pointMapper's problem.
	
	auto const* pointData = static_cast<PointData*>(event.GetClientData());
	const auto point = pointData->getPoint();
	pointWindow->registerPoint(point, pointData->getSelector());	
}

void MainFrame::onUpdatePoint(wxCommandEvent& event)
{
	// This is a message to one of the tabs to add or update a point.
	// Nothing to do with us but to dispatch.

	imageTabFrom->updatePoint(event); // One of them will figure it out.
	imageTabTo->updatePoint(event);
}

void MainFrame::onDeselectWorkingPoint(wxCommandEvent& event)
{
	// This is a message to the PointWindow to get rid of working point and prep for a new one.

	pointWindow->deselectWorkingPoint();
}

void MainFrame::onChangeTab(wxCommandEvent& event)
{
	switch (event.GetInt())
	{
		case 1:
			notebook->SetSelection(0);
			break;
		case 2:
			notebook->SetSelection(1);
			break;
		default:
			break;
	}
}

void MainFrame::onDeleteWorkingPoint(wxCommandEvent& event)
{
	pointWindow->deleteWorkingPoint();
}

void MainFrame::onExportPoints(wxCommandEvent& event)
{
	pointMapper.exportPoints();
}

void MainFrame::onExportAdjustedMap(wxCommandEvent& event)
{
	pointMapper.exportAdjustedMap();
}

void MainFrame::onExit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::onAbout(wxCommandEvent& event)
{
	wxMessageBox(
		 "Copyright (c) 2020 The Paradox Game Converters Group\n\nThis tool, as all others, is free and available at our Github repository.\n\nGithub. Forums. "
		 "Wiki pages. Steam. If you need to find us, report bugs, offer suggestions or wish to participate, we're there.",
		 "Paradox Game Converters Group - Map Adjuster",
		 wxOK | wxICON_INFORMATION);
}

void MainFrame::onSupportUs(wxCommandEvent& event)
{
	wxLaunchDefaultBrowser("https://www.patreon.com/ParadoxGameConverters");
}
