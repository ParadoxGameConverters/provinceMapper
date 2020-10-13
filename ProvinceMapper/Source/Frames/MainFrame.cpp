#include "MainFrame.h"
#include <wx/rawbmp.h>
#include "Log.h"
#include "wx/splitter.h"
#include "ImageFrame.h"

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
	readPixels(ImageTabSelector::SOURCE, img);
	Log(LogLevel::Debug) << "registered " << img.GetSize().GetX() <<"x"<< img.GetSize().GetY() << " source pixels.";
	img.LoadFile("test-to/provinces.bmp", wxBITMAP_TYPE_BMP);
	readPixels(ImageTabSelector::TARGET, img);
	Log(LogLevel::Debug) << "registered " << img.GetSize().GetX() << "x" << img.GetSize().GetY() << " target pixels.";
	
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
	Log(LogLevel::Debug) << "const frame done";
}

void MainFrame::readPixels(ImageTabSelector selector, const wxImage& img)
{
	unsigned char* rgb = img.GetData();
	for (auto y = 0; y < img.GetSize().GetY(); y++)
		for (auto x = 0; x < img.GetSize().GetX(); x++)
		{
			auto border = true;
			// border or regular pixel?
			if (isSameColorAtCoords(x, y, x - 1, y, img) && isSameColorAtCoords(x, y, x + 1, y, img) && isSameColorAtCoords(x, y, x, y - 1, img) &&
				 isSameColorAtCoords(x, y, x, y + 1, img))
				border = false;
			const auto offs = coordsToOffset(x, y, img.GetSize().GetX());

			if (selector == ImageTabSelector::SOURCE && border == true)
				sourceDefs.registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::SOURCE && border == false)
				sourceDefs.registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::TARGET && border == true)
				targetDefs.registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::TARGET && border == false)
				targetDefs.registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
		}
}

bool MainFrame::isSameColorAtCoords(const int ax, const int ay, const int bx, const int by, const wxImage& img)
{
	const auto height = img.GetSize().GetY();
	const auto width = img.GetSize().GetX();
	if (ax > width - 1 || ax < 0 || bx > width - 1 || bx < 0)
		return false;
	if (ay > height - 1 || ay < 0 || by > height - 1 || by < 0)
		return false;
	const auto offsetA = coordsToOffset(ax, ay, width);
	const auto offsetB = coordsToOffset(bx, by, width);
	unsigned char* rgb = img.GetData();
	if (rgb[offsetA] == rgb[offsetB] && rgb[offsetA + 1] == rgb[offsetB + 1] && rgb[offsetA + 2] == rgb[offsetB + 2])
		return true;
	else
		return false;
}

int MainFrame::coordsToOffset(int x, int y, int width)
{
	return (y * width + x) * 3;
}

void MainFrame::initFrame()
{
	Log(LogLevel::Debug) << "init frame start";
	MyFrame* frame = new MyFrame(NULL);
	frame->Show();
	//vbox = new wxFlexGridSizer(1, 2, 0, 0); // This is a "Sizer". It visually defines our application as a grid, of 1 row and 2 columns.
	//notebook = new wxSplitterWindow(this, wxID_ANY); 
	Log(LogLevel::Debug) << "1";

	//imageTabFrom = new ImageTab(notebook, ImageTabSelector::SOURCE, linkMapper.getActiveVersion()); // We create the first image here
	Log(LogLevel::Debug) << "2";
	wxBoxSizer* panel1Sizer = new wxBoxSizer(wxHORIZONTAL);
	Log(LogLevel::Debug) << "3";
	panel1Sizer->Add(imageTabFrom, 1, wxEXPAND);
	Log(LogLevel::Debug) << "4";
	//imageTabFrom->SetSizer(panel1Sizer);
	Log(LogLevel::Debug) << "5";

	//imageTabTo = new ImageTab(notebook, ImageTabSelector::TARGET, linkMapper.getActiveVersion()); // We create the first image here
	wxBoxSizer* panel2Sizer = new wxBoxSizer(wxHORIZONTAL);
	panel2Sizer->Add(imageTabTo, 1, wxEXPAND);
	//imageTabTo->SetSizer(panel2Sizer);
	Log(LogLevel::Debug) << "6";

	//notebook->SplitVertically(imageTabFrom, imageTabTo);
	Log(LogLevel::Debug) << "7";

	//linkWindow = new LinkWindow(this, linkMapper.getActiveVersion()); // Pointwindow goes to the right.
	Log(LogLevel::Debug) << "8";
	//linkWindow->SetMinSize(wxSize(200, 1000));
	Log(LogLevel::Debug) << "9";

	//vbox->Add(linkWindow, wxSizerFlags(0).Expand()); // And finally we register the graphical components with the sizer so they get positioned.
	Log(LogLevel::Debug) << "10";
	//vbox->Add(notebook, wxSizerFlags(1).Expand());
	Log(LogLevel::Debug) << "11";

	//notebook->SetMinSize(wxSize(9000, 5000)); // Since out imageTabs do not contain actual text, but dynamically rendered images on a Device Context, we don't know their size.

	//SetSizer(vbox); // We apply the sizer to mainframe
	Log(LogLevel::Debug) << "2";

	// and have them displayed.
	//imageTabFrom->refresh();
	//imageTabTo->refresh();
	//linkWindow->redrawGrid();
	Log(LogLevel::Debug) << "done";
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
