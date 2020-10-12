#include "MainFrame.h"
#include <wx/rawbmp.h>
#include "Log.h"
#include "wx/splitter.h"

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size): wxFrame(nullptr, wxID_ANY, title, pos, size)
{
	// MainFrame is what holds the entire application together, visually and logically.
	// It receives events from both the visual side (imageTabs and pointWindow), and the logical/file side (pointMapper),
	// and dispatches in response.

	sourceDefs.loadDefinitions("test-from/definition.csv");
	Log(LogLevel::Debug) << "loaded " << sourceDefs.getProvinces().size() << " source provinces.";
	targetDefs.loadDefinitions("test-to/definition.csv");
	Log(LogLevel::Debug) << "loaded " << targetDefs.getProvinces().size() << " target provinces.";
	linkMapper.loadMappings("test_mappings.txt", sourceDefs, targetDefs);
	const auto& activeLinks = linkMapper.getActiveVersion()->getLinks();
	Log(LogLevel::Debug) << "loaded " << activeLinks->size() << " active links.";
	// Import pixels.
	wxImage img;
	img.LoadFile("test-from/provinces.bmp", wxBITMAP_TYPE_BMP);
	unsigned char* rgb = img.GetData();

	for (auto y = 0; y < img.GetSize().GetY(); y++)
		for (auto x = 0; x < img.GetSize().GetX(); x++)
		{
			auto offs = y * img.GetSize().GetX() + x;
			offs *= 3;
			sourceDefs.registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
		}
	Log(LogLevel::Debug) << "registered " << img.GetSize().GetX() <<"x"<< img.GetSize().GetY() << " source pixels.";
	img.LoadFile("test-to/provinces.bmp", wxBITMAP_TYPE_BMP);
	rgb = img.GetData();
	for (auto y = 0; y < img.GetSize().GetY(); y++)
		for (auto x = 0; x < img.GetSize().GetX(); x++)
		{
			auto offs = y * img.GetSize().GetX() + x;
			offs *= 3;
			targetDefs.registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
		}
	Log(LogLevel::Debug) << "registered " << img.GetSize().GetX() <<"x"<< img.GetSize().GetY() << " target pixels.";

	
	Bind(wxEVT_CHANGE_TAB, &MainFrame::onChangeTab, this);
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
}

void MainFrame::initFrame()
{
	vbox = new wxFlexGridSizer(1, 2, 0, 0); // This is a "Sizer". It visually defines our application as a grid, of 1 row and 2 columns.
	notebook = new wxNotebook(this, wxID_ANY); // This is a "Notebook" for the tab layout of the ImageTabs.

	imageTabFrom = new ImageTab(notebook, ImageTabSelector::SOURCE, linkMapper.getActiveVersion()); // We create the first image here
	notebook->AddPage(imageTabFrom, "Source"); // And give it to the notebook
	imageTabTo = new ImageTab(notebook, ImageTabSelector::TARGET, linkMapper.getActiveVersion());
	notebook->AddPage(imageTabTo, "Target");

	linkWindow = new LinkWindow(this, linkMapper.getActiveVersion()); // Pointwindow goes to the right.
	linkWindow->SetMinSize(wxSize(200, 1000));

	vbox->Add(linkWindow, wxSizerFlags(0).Expand()); // And finally we register the graphical components with the sizer so they get positioned.
	vbox->Add(notebook, wxSizerFlags(1).Expand());

	notebook->SetMinSize(wxSize(9000, 5000)); // Since out imageTabs do not contain actual text, but dynamically rendered images on a Device Context, we don't know their size.

	SetSizer(vbox); // We apply the sizer to mainframe

	// and have them displayed.
	imageTabFrom->refresh();
	imageTabTo->refresh();
	linkWindow->redrawGrid();
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
