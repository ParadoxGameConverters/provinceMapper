#include "MainFrame.h"
#include <wx/rawbmp.h>
#include "Log.h"
#include "wx/splitter.h"
#include "Images/ImageFrame.h"
#include "Images/ImageCanvas.h"
#include "Links/LinksFrame.h"

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size): wxFrame(nullptr, wxID_ANY, title, pos, size)
{	
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
}

void MainFrame::initFrame()
{
	auto evt = wxCommandEvent();
	initImageFrame(evt);
	initLinksFrame(evt);
	vbox = new wxFlexGridSizer(1, 2, 0, 0);
	SetSizer(vbox);

}

void MainFrame::initLinksFrame(wxCommandEvent& event)
{
	linksFrame = new LinksFrame(this, linkMapper.getVersions(), linkMapper.getActiveVersion());
	linksFrame->initFrame();
	linksFrame->Show();
}


void MainFrame::initImageFrame(wxCommandEvent& event)
{
	sourceDefs.loadDefinitions("test-from/definition.csv");
	Log(LogLevel::Info) << "Loaded " << sourceDefs.getProvinces().size() << " source provinces.";
	targetDefs.loadDefinitions("test-to/definition.csv");
	Log(LogLevel::Info) << "Loaded " << targetDefs.getProvinces().size() << " target provinces.";
	
	linkMapper.loadMappings("test_mappings.txt", sourceDefs, targetDefs);
	const auto& activeLinks = linkMapper.getActiveVersion()->getLinks();
	Log(LogLevel::Info) << "Loaded " << activeLinks->size() << " active links.";
	
	// Import pixels.
	sourceImg = new wxImage();
	sourceImg->LoadFile("test-from/provinces.bmp", wxBITMAP_TYPE_BMP);
	readPixels(ImageTabSelector::SOURCE, *sourceImg);
	Log(LogLevel::Info) << "Registered " << sourceImg->GetSize().GetX() << "x" << sourceImg->GetSize().GetY() << " source pixels.";

	targetImg = new wxImage();
	targetImg->LoadFile("test-to/provinces.bmp", wxBITMAP_TYPE_BMP);
	readPixels(ImageTabSelector::TARGET, *targetImg);
	Log(LogLevel::Info) << "Registered " << targetImg->GetSize().GetX() << "x" << targetImg->GetSize().GetY() << " target pixels.";
	
	imageFrame = new ImageFrame(this, linkMapper.getActiveVersion(), sourceImg, targetImg);
	wxMenu* menuDropDown = new wxMenu;
	menuDropDown->Append(wxID_REVERT, "Toggle Orientation");
	menuDropDown->Append(wxID_BOLD, "Toggle The Black");
	wxMenuBar* imageMenuBar = new wxMenuBar;
	imageMenuBar->Append(menuDropDown, "&Image");
	imageFrame->SetMenuBar(imageMenuBar);

	imageFrame->Show();
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
