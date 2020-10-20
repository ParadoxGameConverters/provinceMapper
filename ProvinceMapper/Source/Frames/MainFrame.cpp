#include "MainFrame.h"
#include "CommonFunctions.h"
#include "Images/ImageCanvas.h"
#include "Images/ImageFrame.h"
#include "Links/DialogComment.h"
#include "Links/LinksFrame.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include "wx/splitter.h"
#include <fstream>
#include <wx/filepicker.h>
#include <wx/rawbmp.h>

wxDEFINE_EVENT(wxMENU_ADD_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_ADD_COMMENT, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_ADD_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_COPY_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_DELETE_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_RENAME_VERSION, wxCommandEvent);

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size): wxFrame(nullptr, wxID_ANY, title, pos, size)
{
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
	Bind(wxEVT_MENU, &MainFrame::onSaveLinks, this, wxID_SAVE);
	Bind(wxEVT_MENU, &MainFrame::onLinksAddLink, this, wxMENU_ADD_LINK);
	Bind(wxEVT_MENU, &MainFrame::onDeleteActiveLink, this, wxEVT_DELETE_ACTIVE_LINK);
	Bind(wxEVT_MENU, &MainFrame::onLinksAddComment, this, wxMENU_ADD_COMMENT);
	Bind(wxEVT_MENU, &MainFrame::onVersionsAddVersion, this, wxMENU_ADD_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsCopyVersion, this, wxMENU_COPY_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsDeleteVersion, this, wxMENU_DELETE_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsRenameVersion, this, wxMENU_RENAME_VERSION);

	Bind(wxEVT_DEACTIVATE_LINK, &MainFrame::onDeactivateLink, this);
	Bind(wxEVT_DELETE_ACTIVE_LINK, &MainFrame::onDeleteActiveLink, this);
	Bind(wxEVT_SELECT_LINK_BY_INDEX, &MainFrame::onActivateLinkByIndex, this);
	Bind(wxEVT_SELECT_LINK_BY_ID, &MainFrame::onActivateLinkByID, this);
	Bind(wxEVT_TOGGLE_PROVINCE, &MainFrame::onToggleProvince, this);
	Bind(wxEVT_CENTER_MAP, &MainFrame::onCenterMap, this);
	Bind(wxEVT_ADD_COMMENT, &MainFrame::onAddComment, this);
	Bind(wxEVT_UPDATE_NAME, &MainFrame::onRenameVersion, this);
	Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MainFrame::onChangeTab, this);
}

void MainFrame::initFrame()
{
	configuration.load();
	SetBackgroundColour(wxColour(230, 230, 230));
	sizer = new wxFlexGridSizer(4, 3, 5);
	SetSizer(sizer);

	// Source directory
	auto* sdirText = new wxStaticText(this, wxID_ANY, "Source Directory", wxDefaultPosition);
	sourceDirPicker = new wxDirPickerCtrl(this, 0, wxEmptyString, "BROWSE", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	sourceDirPicker->Bind(wxEVT_DIRPICKER_CHANGED, &MainFrame::onPathChanged, this);
	sourceDirStatus = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	reverseSourceCheck = new wxCheckBox(this, 0, "Reverse?", wxDefaultPosition, wxDefaultSize, wxEXPAND, wxDefaultValidator);
	if (configuration.isSourceReversed())
		reverseSourceCheck->SetValue(true);
	reverseSourceCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (reverseSourceCheck->GetValue())
			configuration.setSourceReversed(true);
		else
			configuration.setSourceReversed(false);
		configuration.save();
	});

	sizer->Add(sdirText, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->Add(sourceDirPicker, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5);
	sizer->Add(sourceDirStatus, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->Add(reverseSourceCheck, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

	// Target Directory
	auto* tdirText = new wxStaticText(this, wxID_ANY, "Target Directory", wxDefaultPosition);
	targetDirPicker = new wxDirPickerCtrl(this, 1, wxEmptyString, "BROWSE", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	targetDirPicker->Bind(wxEVT_DIRPICKER_CHANGED, &MainFrame::onPathChanged, this);
	targetDirStatus = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	reverseTargetCheck = new wxCheckBox(this, 0, "Reverse?", wxDefaultPosition, wxDefaultSize, wxEXPAND, wxDefaultValidator);
	if (configuration.isTargetReversed())
		reverseTargetCheck->SetValue(true);
	reverseTargetCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (reverseTargetCheck->GetValue())
			configuration.setTargetReversed(true);
		else
			configuration.setTargetReversed(false);
		configuration.save();
	});

	sizer->Add(tdirText, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(targetDirPicker, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(targetDirStatus, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->Add(reverseTargetCheck, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

	// Link File
	auto* linkFileText = new wxStaticText(this, wxID_ANY, "Link File", wxDefaultPosition);
	linkFilePicker =
		 new wxFilePickerCtrl(this, 2, wxEmptyString, "BROWSE", "*.txt", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	linkFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, &MainFrame::onPathChanged, this);
	linkFileStatus = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	sizer->Add(linkFileText, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(linkFilePicker, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(linkFileStatus, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->AddStretchSpacer(0);

	// Source Token
	auto* sourceTokenText = new wxStaticText(this, wxID_ANY, "Source Token", wxDefaultPosition);
	sourceTokenField = new wxTextCtrl(this, 3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_DEFAULT);
	sourceTokenField->Bind(wxEVT_TEXT, &MainFrame::onTokenChanged, this);
	sourceTokenStatus = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	sizer->Add(sourceTokenText, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(sourceTokenField, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(sourceTokenStatus, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->AddStretchSpacer(0);

	// Target Token
	auto* targetTokenText = new wxStaticText(this, wxID_ANY, "Target Token", wxDefaultPosition);
	targetTokenField = new wxTextCtrl(this, 4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_DEFAULT);
	targetTokenField->Bind(wxEVT_TEXT, &MainFrame::onTokenChanged, this);
	targetTokenStatus = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	sizer->Add(targetTokenText, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(targetTokenField, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5, nullptr);
	sizer->Add(targetTokenStatus, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer->AddStretchSpacer(0);

	// The Button
	startButton = new wxButton(this, wxID_ANY, "Begin!", wxDefaultPosition, wxDefaultSize);
	startButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::onStartButton, this);
	startButton->Disable();

	sizer->AddStretchSpacer(0);
	sizer->Add(startButton, wxSizerFlags(1).Top().CenterHorizontal());
	sizer->AddStretchSpacer(0);
	sizer->AddStretchSpacer(0);

	populateFrame();
}

void MainFrame::populateFrame()
{
	std::string path;
	std::wstring path16;

	// Source path
	if (configuration.getSourceDir())
	{
		path = *configuration.getSourceDir();
		path16 = commonItems::convertUTF8ToUTF16(path);
	}
	sourceDirPicker->SetPath(path16);
	sourceDirPicker->SetInitialDirectory(path16);
	auto bootEvent0 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 0, path16);
	onPathChanged(bootEvent0);

	// target path
	if (configuration.getTargetDir())
	{
		path = *configuration.getTargetDir();
		path16 = commonItems::convertUTF8ToUTF16(path);
	}
	targetDirPicker->SetPath(path16);
	targetDirPicker->SetInitialDirectory(path16);
	auto bootEvent1 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 1, path16);
	onPathChanged(bootEvent1);

	// source token
	std::string token;
	if (configuration.getSourceToken())
		token = *configuration.getSourceToken();
	sourceTokenField->SetValue(token);
	auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
	onTokenChanged(bootEvent3);

	// target token
	if (configuration.getTargetToken())
		token = *configuration.getTargetToken();
	targetTokenField->SetValue(token);
	auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
	onTokenChanged(bootEvent4);

	// links file goes last, because it can overwrite tokens if those aren't already loaded.
	std::wstring initialPath;
	if (configuration.getLinkFile())
	{
		path = *configuration.getLinkFile();
		path16 = commonItems::convertUTF8ToUTF16(path);
		const auto rawFile = trimPath(path);
		const auto rawPath = path.substr(0, rawFile.size());
		initialPath = commonItems::convertUTF8ToUTF16(rawPath);
	}
	linkFilePicker->SetPath(path16);
	linkFilePicker->SetInitialDirectory(initialPath);
	auto bootEvent2 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 2, path16);
	onPathChanged(bootEvent2);
}

void MainFrame::initLinksFrame()
{
	linksFrame = new LinksFrame(this, linkMapper.getVersions(), linkMapper.getActiveVersion());
	auto* saveDropDown = new wxMenu;
	saveDropDown->Append(wxID_SAVE);
	auto* linksDropDown = new wxMenu;
	linksDropDown->Append(wxMENU_ADD_LINK, "Add Link\tCtrl-L");
	linksDropDown->Append(wxMENU_ADD_COMMENT, "Add Comment\tCtrl-C");
	linksDropDown->Append(wxEVT_DELETE_ACTIVE_LINK, "Delete Selected\tCtrl-D");
	auto* versionsDropDown = new wxMenu;
	versionsDropDown->Append(wxMENU_ADD_VERSION, "New Version");
	versionsDropDown->Append(wxMENU_COPY_VERSION, "Copy Version");
	versionsDropDown->Append(wxMENU_RENAME_VERSION, "Rename Version");
	versionsDropDown->Append(wxMENU_DELETE_VERSION, "Delete (Danger!)");

	auto* linksMenuBar = new wxMenuBar;
	linksMenuBar->Append(saveDropDown, "&File");
	linksMenuBar->Append(linksDropDown, "&Links/Comments");
	linksMenuBar->Append(versionsDropDown, "&Versions");

	linksFrame->SetMenuBar(linksMenuBar);
	linksFrame->Show();
}

void MainFrame::initImageFrame()
{
	localizationMapper.scrapeSourceDir(*configuration.getSourceDir());
	Log(LogLevel::Info) << "Source Localizations Loaded.";
	localizationMapper.scrapeTargetDir(*configuration.getTargetDir());
	Log(LogLevel::Info) << "Target Localizations Loaded.";

	sourceDefs = std::make_shared<Definitions>();
	targetDefs = std::make_shared<Definitions>();

	sourceDefs->loadDefinitions(*configuration.getSourceDir() + "/definition.csv", localizationMapper, LocalizationMapper::LocType::SOURCE);
	Log(LogLevel::Info) << "Loaded " << sourceDefs->getProvinces().size() << " source provinces.";
	targetDefs->loadDefinitions(*configuration.getTargetDir() + "/definition.csv", localizationMapper, LocalizationMapper::LocType::TARGET);
	Log(LogLevel::Info) << "Loaded " << targetDefs->getProvinces().size() << " target provinces.";

	linkMapper.loadMappings(linksFileString, sourceDefs, targetDefs, *configuration.getSourceToken(), *configuration.getTargetToken());
	const auto& activeLinks = linkMapper.getActiveVersion()->getLinks();
	Log(LogLevel::Info) << "Loaded " << activeLinks->size() << " active links.";

	// Import pixels.
	wxLogNull AD; // disable warning about proprietary and thus unsupported sRGB profiles in PDX PNGs.
	sourceImg = new wxImage();
	sourceRiversImg = new wxImage();
	if (commonItems::DoesFileExist(*configuration.getSourceDir() + "/provinces.bmp"))
		sourceImg->LoadFile(*configuration.getSourceDir() + "/provinces.bmp");
	else if (commonItems::DoesFileExist(*configuration.getSourceDir() + "/provinces.png"))
		sourceImg->LoadFile(*configuration.getSourceDir() + "/provinces.png");
	if (commonItems::DoesFileExist(*configuration.getSourceDir() + "/rivers.bmp"))
		sourceRiversImg->LoadFile(*configuration.getSourceDir() + "/rivers.bmp");
	else if (commonItems::DoesFileExist(*configuration.getSourceDir() + "/rivers.png"))
		sourceRiversImg->LoadFile(*configuration.getSourceDir() + "/rivers.png");

	targetImg = new wxImage();
	targetRiversImg = new wxImage();
	if (commonItems::DoesFileExist(*configuration.getTargetDir() + "/provinces.bmp"))
		targetImg->LoadFile(*configuration.getTargetDir() + "/provinces.bmp");
	else if (commonItems::DoesFileExist(*configuration.getTargetDir() + "/provinces.png"))
		targetImg->LoadFile(*configuration.getTargetDir() + "/provinces.png");
	if (commonItems::DoesFileExist(*configuration.getTargetDir() + "/rivers.bmp"))
		targetRiversImg->LoadFile(*configuration.getTargetDir() + "/rivers.bmp");
	else if (commonItems::DoesFileExist(*configuration.getTargetDir() + "/rivers.png"))
		targetRiversImg->LoadFile(*configuration.getTargetDir() + "/rivers.png");

	mergeRivers();

	if (configuration.isSourceReversed())
		*sourceImg = sourceImg->Mirror(false);
	readPixels(ImageTabSelector::SOURCE, *sourceImg);
	Log(LogLevel::Info) << "Registered " << sourceImg->GetSize().GetX() << "x" << sourceImg->GetSize().GetY() << " source pixels.";

	if (configuration.isTargetReversed())
		*targetImg = targetImg->Mirror(false);
	readPixels(ImageTabSelector::TARGET, *targetImg);
	Log(LogLevel::Info) << "Registered " << targetImg->GetSize().GetX() << "x" << targetImg->GetSize().GetY() << " target pixels.";

	imageFrame = new ImageFrame(this, linkMapper.getActiveVersion(), sourceImg, targetImg, sourceDefs, targetDefs);
	auto* menuDropDown = new wxMenu;
	menuDropDown->Append(wxID_REVERT, "Toggle Orientation");
	menuDropDown->Append(wxID_BOLD, "Toggle The Shade");
	auto* imageMenuBar = new wxMenuBar;
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

void MainFrame::readPixels(const ImageTabSelector selector, const wxImage& img)
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
				sourceDefs->registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::SOURCE && border == false)
				sourceDefs->registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::TARGET && border == true)
				targetDefs->registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else if (selector == ImageTabSelector::TARGET && border == false)
				targetDefs->registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
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

	// Override for river colors which are hardcoded at 200/200/200. They are always true so adjacent pixels are not border pixels.
	if (rgb[offsetA] == 200 && rgb[offsetA + 1] == 200 && rgb[offsetA + 2] == 200)
		return true;
	if (rgb[offsetB] == 200 && rgb[offsetB + 1] == 200 && rgb[offsetB + 2] == 200)
		return true;

	// Otherwise compare them normally.
	if (rgb[offsetA] == rgb[offsetB] && rgb[offsetA + 1] == rgb[offsetB + 1] && rgb[offsetA + 2] == rgb[offsetB + 2])
		return true;
	else
		return false;
}

void MainFrame::onPathChanged(wxFileDirPickerEvent& evt)
{
	const auto validPath = commonItems::DoesFolderExist(commonItems::UTF16ToUTF8(evt.GetPath().ToStdWstring()));
	const auto result = commonItems::UTF16ToUTF8(evt.GetPath().ToStdWstring());
	const auto green = wxColour(130, 250, 130);
	const auto red = wxColour(250, 130, 130);

	// source path
	if (evt.GetId() == 0)
	{
		if (validPath && (commonItems::DoesFileExist(result + "/provinces.bmp") || commonItems::DoesFileExist(result + "/provinces.png")))
		{
			sourceDirStatus->SetBackgroundColour(green);
			sanity[0] = true;
		}
		else
		{
			sourceDirStatus->SetBackgroundColour(red);
			sanity[0] = false;
		}
		configuration.setSourceDir(result);
	}
	// target path
	else if (evt.GetId() == 1)
	{
		if (validPath && (commonItems::DoesFileExist(result + "/provinces.bmp") || commonItems::DoesFileExist(result + "/provinces.png")))
		{
			targetDirStatus->SetBackgroundColour(green);
			sanity[1] = true;
		}
		else
		{
			targetDirStatus->SetBackgroundColour(red);
			sanity[1] = false;
		}
		configuration.setTargetDir(result);
	}
	// links file
	else if (evt.GetId() == 2)
	{
		if (commonItems::DoesFileExist(result))
		{
			linkFileStatus->SetBackgroundColour(green);
			std::ifstream linksFile(result);
			if (linksFile.is_open())
				linksFileString.assign(std::istreambuf_iterator<char>(linksFile), std::istreambuf_iterator<char>());
			linksFile.close();
			sanity[2] = true;
			auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
			onTokenChanged(bootEvent3);
			auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
			onTokenChanged(bootEvent4);
		}
		else if (result.empty())
		{
			// We allow for nothing.
			linkFileStatus->SetBackgroundColour(green);
			linksFileString.clear();
			sanity[2] = true;
			auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
			onTokenChanged(bootEvent3);
			auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
			onTokenChanged(bootEvent4);
		}
		else
		{
			linkFileStatus->SetBackgroundColour(red);
			linksFileString.clear();
			sanity[2] = false;
			auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
			onTokenChanged(bootEvent3);
			auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
			onTokenChanged(bootEvent4);
		}
		configuration.setLinkFile(result);
	}
	configuration.save();
	applySanityToButton();
	Refresh();
}

void MainFrame::onTokenChanged(wxCommandEvent& evt)
{
	const auto green = wxColour(130, 250, 130);
	const auto red = wxColour(250, 130, 130);

	// source token
	if (evt.GetId() == 3)
	{
		const auto input = sourceTokenField->GetValue();
		const auto rawinput = commonItems::UTF16ToUTF8(input.ToStdWstring());
		if (!rawinput.empty() && rawinput.size() >= 2 && (linksFileString.find(rawinput) != std::string::npos || linksFileString.empty()))
		{
			sourceTokenStatus->SetBackgroundColour(green);
			sanity[3] = true;
		}
		else
		{
			sourceTokenStatus->SetBackgroundColour(red);
			sanity[3] = false;
		}
		configuration.setSourceToken(rawinput);
	}
	// target token
	else if (evt.GetId() == 4)
	{
		const auto input = targetTokenField->GetValue();
		const auto rawinput = commonItems::UTF16ToUTF8(input.ToStdWstring());
		if (!rawinput.empty() && rawinput.size() >= 2 && (linksFileString.find(rawinput) != std::string::npos || linksFileString.empty()))
		{
			targetTokenStatus->SetBackgroundColour(green);
			sanity[4] = true;
		}
		else
		{
			targetTokenStatus->SetBackgroundColour(red);
			sanity[4] = false;
		}
		configuration.setTargetToken(rawinput);
	}
	configuration.save();
	applySanityToButton();
	Refresh();
}

void MainFrame::applySanityToButton()
{
	if (sanity[0] && sanity[1] && sanity[2] && sanity[3] && sanity[4])
		startButton->Enable();
	else
		startButton->Disable();
}

void MainFrame::onStartButton(wxCommandEvent& evt)
{
	startButton->Disable();
	Refresh();
	initImageFrame();
	initLinksFrame();
	Hide();
}

void MainFrame::onSaveLinks(wxCommandEvent& evt)
{
	if (!configuration.getLinkFile()->empty())
		linkMapper.exportMappings(*configuration.getLinkFile());
	else
		linkMapper.exportMappings("province_mappings.txt");
	wxMessageBox("Links saved.", "Save");
}

void MainFrame::onDeactivateLink(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();
}

void MainFrame::onActivateLinkByIndex(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();
	linkMapper.activateLinkByIndex(evt.GetInt());
	linksFrame->activateLinkByIndex(evt.GetInt());
	imageFrame->activateLinkByIndex(evt.GetInt());
}

void MainFrame::onActivateLinkByID(wxCommandEvent& evt)
{
	// This is coming from one of the imageCanvases, so we need to update everything.
	linkMapper.deactivateLink();
	imageFrame->deactivateLink();
	linksFrame->deactivateLink();
	linkMapper.activateLinkByID(evt.GetInt());
	imageFrame->activateLinkByID(evt.GetInt());
	linksFrame->activateLinkByID(evt.GetInt());
}

void MainFrame::onToggleProvince(wxCommandEvent& evt)
{
	// Two things can happen. We're either:
	// 1. toggling a province within our active link
	// 2. toggling a province without active link, thus creating one.
	// In the second case we need to update quite a lot of things.

	const auto ID = std::abs(evt.GetInt());
	const auto sourceImage = evt.GetInt() > 0;

	const auto newLinkID = linkMapper.toggleProvinceByID(ID, sourceImage);
	if (!newLinkID)
	{
		linksFrame->refreshActiveLink();
		imageFrame->toggleProvinceByID(ID, sourceImage);
	}
	else
	{
		linksFrame->createLink(*newLinkID);
		imageFrame->shadeProvinceByID(ID, sourceImage);
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::onCenterMap(wxCommandEvent& evt)
{
	imageFrame->centerMap(evt.GetInt());
}

void MainFrame::onAddComment(wxCommandEvent& evt)
{
	const auto newLinkID = linkMapper.addCommentByIndex(evt.GetString().ToStdString(), evt.GetInt());
	if (newLinkID)
	{
		linksFrame->createLink(*newLinkID);
		imageFrame->deactivateLink();
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::onDeleteActiveLink(wxCommandEvent& evt)
{
	// We don't need an ID since this works only on active link.
	imageFrame->deleteActiveLink(); // images first so it knows which provinces to recolor.
	linkMapper.deleteActiveLink();
	linksFrame->deactivateLink();
}

void MainFrame::onLinksAddLink(wxCommandEvent& evt)
{
	// This one creates a link without any provinces.
	const auto newLinkID = linkMapper.addRawLink();
	if (newLinkID)
	{
		linksFrame->createLink(*newLinkID);
		imageFrame->deactivateLink();
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::onLinksAddComment(wxCommandEvent& evt)
{
	// This one creates a blank comment at unspecified location.
	const auto newLinkID = linkMapper.addRawComment();
	if (newLinkID)
	{
		linksFrame->createLink(*newLinkID);
		imageFrame->deactivateLink();
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::mergeRivers()
{
	if (sourceRiversImg->IsOk())
	{
		Log(LogLevel::Info) << "Merging source rivers";
		auto* imageData = sourceImg->GetData();
		const auto imageDataSize = sourceImg->GetSize().x * sourceImg->GetSize().y * 3;
		auto* riverData = sourceRiversImg->GetData();
		const auto riverDataSize = sourceRiversImg->GetSize().x * sourceRiversImg->GetSize().y * 3;
		if (riverDataSize == imageDataSize)
			mergeRiverData(imageData, riverData, imageDataSize);
	}
	if (targetRiversImg->IsOk())
	{
		Log(LogLevel::Info) << "Merging target rivers";
		auto* imageData = targetImg->GetData();
		const auto imageDataSize = targetImg->GetSize().x * targetImg->GetSize().y * 3;
		auto* riverData = targetRiversImg->GetData();
		const auto riverDataSize = targetRiversImg->GetSize().x * targetRiversImg->GetSize().y * 3;
		if (riverDataSize == imageDataSize)
			mergeRiverData(imageData, riverData, imageDataSize);
	}
}

void MainFrame::mergeRiverData(unsigned char* imgData, unsigned char* riverData, const int size)
{
	for (auto offset = 0; offset < size; offset = offset + 3)
	{
		if (!isRiverMask(riverData[offset], riverData[offset + 1], riverData[offset + 2]))
		{
			// We're using 200/200/200 for river color because it's a tolerable shade not usually used for any province.
			// (PDX appears not to be using shades of gray for provinces).
			// We're overwriting original map's pixels with this, so those pixels will not be a part of any defined province.
			// However, as undefined pixels are unmappable, no harm done.
			// Still, user beware.
			imgData[offset] = 200;
			imgData[offset + 1] = 200;
			imgData[offset + 2] = 200;
		}
	}
}

bool MainFrame::isRiverMask(const unsigned char r, const unsigned char g, const unsigned char b)
{
	// eu4 and hoi4 use gray, ck2/3/vic2 use pink for sea, white is always land.
	if (r == 255 && g == 0 && b == 128 || r == 122 && g == 122 && b == 122 || r == 255 && g == 255 && b == 255)
		return true;
	else
		return false;
}

void MainFrame::onVersionsAddVersion(wxCommandEvent& evt)
{
	// Turn off any active links.
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	// Create new version
	const auto& newVersion = linkMapper.addVersion();
	linksFrame->addVersion(newVersion);
	imageFrame->setVersion(newVersion);
}

void MainFrame::onVersionsCopyVersion(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& newVersion = linkMapper.copyVersion();
	linksFrame->addVersion(newVersion);
	imageFrame->setVersion(newVersion);
}

void MainFrame::onVersionsDeleteVersion(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& activeVersion = linkMapper.deleteVersion();
	linksFrame->deleteActiveAndSwapToVersion(activeVersion);
	imageFrame->setVersion(activeVersion);
}

void MainFrame::onVersionsRenameVersion(wxCommandEvent& evt)
{
	const auto& version = linkMapper.getActiveVersion();
	if (version)
	{
		auto* dialog = new DialogComment(this, "Edit Name", version->getName(), version->getID());
		dialog->ShowModal();
	}
}

void MainFrame::onRenameVersion(wxCommandEvent& evt)
{
	const auto versionName = evt.GetString().ToStdString();

	linkMapper.updateActiveVersionName(versionName);
	linksFrame->updateActiveVersionName(versionName);
}

void MainFrame::onChangeTab(wxBookCtrlEvent& event)
{
	// linksTab has changed tab to another set. We need to update everything.

	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& activeVersion = linkMapper.activateVersionByIndex(event.GetSelection());
	linksFrame->setVersion(activeVersion);
	imageFrame->setVersion(activeVersion);
}
