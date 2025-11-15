#include "MainFrame.h"
#include "CommonFunctions.h"
#include "Definitions/Definitions.h"
#include "Definitions/EU5Definitions.h"
#include "Definitions/Vic3Definitions.h"
#include "Images/ImageCanvas.h"
#include "Images/ImageFrame.h"
#include "Links/DialogComment.h"
#include "Links/LinksFrame.h"
#include "Links/ProvinceMappingsGrid.h"
#include "Links/TriangulationPairsGrid.h"
#include "PixelReader/PixelReader.h"
#include "Search/SearchFrame.h"
#include "Unmapped/UnmappedFrame.h"
#include "Unmapped/UnmappedTab.h"
#include <Log.h>
#include <OSCompatibilityLayer.h>
#include <fstream>
#include <wx/filepicker.h>
#include <wx/rawbmp.h>
#include <wx/splitter.h>

wxDEFINE_EVENT(wxMENU_ADD_COMMENT, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_ADD_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_COPY_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_DELETE_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_RENAME_VERSION, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_SHOW_TOOLBAR, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_SHOW_UNMAPPED, wxCommandEvent);
wxDEFINE_EVENT(wxMENU_SHOW_SEARCH, wxCommandEvent);

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size):
	 wxFrame(nullptr, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
	Bind(wxEVT_MENU, &MainFrame::onExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::onAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::onSupportUs, this, wxID_NETWORK);
	Bind(wxEVT_MENU, &MainFrame::onSaveLinks, this, wxID_SAVE);
	Bind(wxEVT_MENU, &MainFrame::onLinksAddLink, this, wxEVT_ADD_LINK);
	Bind(wxEVT_MENU, &MainFrame::onLinksAddTriangulationPair, this, wxEVT_ADD_TRIANGULATION_PAIR);
	Bind(wxEVT_MENU, &MainFrame::onAutogenerateMappings, this, wxEVT_AUTOGENERATE_MAPPINGS);
	Bind(wxEVT_MENU, &MainFrame::onDeleteActiveLinkOrTriangulationPair, this, wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR);
	Bind(wxEVT_MENU, &MainFrame::onLinksAddComment, this, wxMENU_ADD_COMMENT);
	Bind(wxEVT_MENU, &MainFrame::onVersionsAddVersion, this, wxMENU_ADD_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsCopyVersion, this, wxMENU_COPY_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsDeleteVersion, this, wxMENU_DELETE_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onVersionsRenameVersion, this, wxMENU_RENAME_VERSION);
	Bind(wxEVT_MENU, &MainFrame::onLinksMoveUp, this, wxEVT_MOVE_ACTIVE_LINK_UP);
	Bind(wxEVT_MENU, &MainFrame::onLinksMoveDown, this, wxEVT_MOVE_ACTIVE_LINK_DOWN);
	Bind(wxEVT_MENU, &MainFrame::onLinksMoveVersionLeft, this, wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	Bind(wxEVT_MENU, &MainFrame::onLinksMoveVersionRight, this, wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	Bind(wxEVT_MENU, &MainFrame::onShowToolbar, this, wxMENU_SHOW_TOOLBAR);
	Bind(wxEVT_MENU, &MainFrame::onShowUnmapped, this, wxMENU_SHOW_UNMAPPED);
	Bind(wxEVT_MENU, &MainFrame::onShowSearch, this, wxMENU_SHOW_SEARCH);

	Bind(wxEVT_DEACTIVATE_LINK, &MainFrame::onDeactivateLink, this);
	Bind(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, &MainFrame::onDeactivateTriangulationPair, this);
	Bind(wxEVT_DELETE_ACTIVE_LINK, &MainFrame::onDeleteActiveLink, this);
	Bind(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR, &MainFrame::onDeleteActiveTriangulationPair, this);
	Bind(wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR, &MainFrame::onDeleteActiveLinkOrTriangulationPair, this);
	Bind(wxEVT_SELECT_LINK_BY_INDEX, &MainFrame::onActivateLinkByIndex, this);
	Bind(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX, &MainFrame::onActivateTriangulationPairByIndex, this);
	Bind(wxEVT_SELECT_LINK_BY_ID, &MainFrame::onActivateLinkByID, this);
	Bind(wxEVT_SELECT_TRIANGULATION_PAIR_BY_ID, &MainFrame::onActivateTriangulationPairByID, this);
	Bind(wxEVT_TOGGLE_PROVINCE, &MainFrame::onToggleProvince, this);
	Bind(wxEVT_CENTER_MAP, &MainFrame::onCenterMap, this);
	Bind(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, &MainFrame::onCenterMapToTriangulationPair, this);
	Bind(wxEVT_ADD_COMMENT, &MainFrame::onAddComment, this);
	Bind(wxEVT_UPDATE_NAME, &MainFrame::onRenameVersion, this);
	Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &MainFrame::onChangeTab, this);
	Bind(wxEVT_MOVE_ACTIVE_LINK_UP, &MainFrame::onLinksMoveUp, this);
	Bind(wxEVT_MOVE_ACTIVE_LINK_DOWN, &MainFrame::onLinksMoveDown, this);
	Bind(wxEVT_SAVE_LINKS, &MainFrame::onSaveLinks, this);
	Bind(wxEVT_ADD_LINK, &MainFrame::onLinksAddLink, this);
	Bind(wxEVT_ADD_TRIANGULATION_PAIR, &MainFrame::onLinksAddTriangulationPair, this);
	Bind(wxEVT_AUTOGENERATE_MAPPINGS, &MainFrame::onAutogenerateMappings, this);
	Bind(wxEVT_MOVE_ACTIVE_VERSION_LEFT, &MainFrame::onLinksMoveVersionLeft, this);
	Bind(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, &MainFrame::onLinksMoveVersionRight, this);
	Bind(wxEVT_PROVINCE_CENTER_MAP, &MainFrame::onCenterProvince, this);
	Bind(wxEVT_REFRESH_ACTIVE_TRIANGULATION_PAIR, &MainFrame::onRefreshActiveTriangulationPair, this);
}

void MainFrame::initFrame()
{
	configuration = std::make_shared<Configuration>();
	configuration->load();
	auto* holderPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxEXPAND);
	holderPanel->SetBackgroundColour(wxColour(230, 230, 230));
	sizer = new wxFlexGridSizer(4, 3, 5);
	holderPanel->SetSizer(sizer);

	// Source directory
	auto* sdirText = new wxStaticText(holderPanel, wxID_ANY, "Source Directory", wxDefaultPosition);
	sourceDirPicker =
		 new wxDirPickerCtrl(holderPanel, 0, wxEmptyString, "BROWSE", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	sourceDirPicker->Bind(wxEVT_DIRPICKER_CHANGED, &MainFrame::onPathChanged, this);
	sourceDirStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	reverseSourceCheck = new wxCheckBox(holderPanel, 0, "Reverse?", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	if (configuration->isSourceReversed())
		reverseSourceCheck->SetValue(true);
	reverseSourceCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (reverseSourceCheck->GetValue())
			configuration->setSourceReversed(true);
		else
			configuration->setSourceReversed(false);
		configuration->save();
	});

	sizer->Add(sdirText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sourceDirPicker, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5).Center());
	sizer->Add(sourceDirStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5).Center());
	sizer->Add(reverseSourceCheck, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT | wxTOP, 5).Center());

	// Target Directory
	auto* tdirText = new wxStaticText(holderPanel, wxID_ANY, "Target Directory", wxDefaultPosition);
	targetDirPicker =
		 new wxDirPickerCtrl(holderPanel, 1, wxEmptyString, "BROWSE", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	targetDirPicker->Bind(wxEVT_DIRPICKER_CHANGED, &MainFrame::onPathChanged, this);
	targetDirStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));
	reverseTargetCheck = new wxCheckBox(holderPanel, 0, "Reverse?", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	if (configuration->isTargetReversed())
		reverseTargetCheck->SetValue(true);
	reverseTargetCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (reverseTargetCheck->GetValue())
			configuration->setTargetReversed(true);
		else
			configuration->setTargetReversed(false);
		configuration->save();
	});

	sizer->Add(tdirText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(targetDirPicker, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(targetDirStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(reverseTargetCheck, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());

	// Link File
	auto* linkFileText = new wxStaticText(holderPanel, wxID_ANY, "Link File", wxDefaultPosition);
	linkFilePicker =
		 new wxFilePickerCtrl(holderPanel, 2, wxEmptyString, "BROWSE", "*.txt", wxDefaultPosition, wxSize(350, wxDefaultCoord), wxFLP_USE_TEXTCTRL | wxFLP_SMALL);
	linkFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, &MainFrame::onPathChanged, this);
	linkFileStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	// Ditch adjacencies check
	ditchCheck = new wxCheckBox(holderPanel, 0, "Ditch Adjacencies?", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	if (configuration->isDitchAdjacencies())
		ditchCheck->SetValue(true);
	ditchCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (ditchCheck->GetValue())
			configuration->setDitchAdjacencies(true);
		else
			configuration->setDitchAdjacencies(false);
		configuration->save();
	});

	sizer->Add(linkFileText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(linkFilePicker, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(linkFileStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(ditchCheck, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());

	// Source Token
	auto* sourceTokenText = new wxStaticText(holderPanel, wxID_ANY, "Source Token", wxDefaultPosition);
	sourceTokenField = new wxTextCtrl(holderPanel, 3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_DEFAULT);
	sourceTokenField->Bind(wxEVT_TEXT, &MainFrame::onTokenChanged, this);
	sourceTokenStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	// "Generate triangulation pairs for source map corners" check
	generateTriangulationPairsForSourceMapCornersCheck = new wxCheckBox(holderPanel,
	 0,
	 "Generate triangulation pairs\nfor source map corners?",
		 wxDefaultPosition,
		 wxDefaultSize,
		 wxCHK_2STATE,
		 wxDefaultValidator);
	if (configuration->isGenerateTriangulationPairsForSourceMapCorners())
		generateTriangulationPairsForSourceMapCornersCheck->SetValue(true);
	generateTriangulationPairsForSourceMapCornersCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (generateTriangulationPairsForSourceMapCornersCheck->GetValue())
			configuration->setGenerateTriangulationPairsForSourceMapCorners(true);
		else
			configuration->setGenerateTriangulationPairsForSourceMapCorners(false);
		configuration->save();
	});

	sizer->Add(sourceTokenText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(sourceTokenField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(sourceTokenStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(generateTriangulationPairsForSourceMapCornersCheck, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());

	// Target Token
	auto* targetTokenText = new wxStaticText(holderPanel, wxID_ANY, "Target Token", wxDefaultPosition);
	targetTokenField = new wxTextCtrl(holderPanel, 4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_DEFAULT);
	targetTokenField->Bind(wxEVT_TEXT, &MainFrame::onTokenChanged, this);
	targetTokenStatus = new wxWindow(holderPanel, wxID_ANY, wxDefaultPosition, wxSize(15, 15));

	sizer->Add(targetTokenText, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(targetTokenField, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->Add(targetTokenStatus, wxSizerFlags(0).Align(wxVERTICAL).Border(wxLEFT | wxRIGHT, 5).Center());
	sizer->AddStretchSpacer(0);

	// The Button
	startButton = new wxButton(holderPanel, wxID_ANY, "Begin!", wxDefaultPosition, wxDefaultSize, wxEXPAND);
	startButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::onStartButton, this);
	startButton->Disable();

	sizer->AddStretchSpacer(0);
	sizer->Add(startButton, wxSizerFlags(1).Expand().Top().CenterHorizontal().Border(wxBOTTOM, 10).Center());
	sizer->AddStretchSpacer(0);
	sizer->AddStretchSpacer(0);

	auto* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(holderPanel, wxSizerFlags(1).Align(wxCENTER).Expand());
	SetSizer(boxSizer);

	populateFrame();
}

void MainFrame::populateFrame()
{
	std::filesystem::path path;

	// Source path
	if (configuration->getSourceDir())
	{
		path = *configuration->getSourceDir();
	}
	sourceDirPicker->SetPath(path.string());
	sourceDirPicker->SetInitialDirectory(path.string());
	auto bootEvent0 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 0, path.string());
	onPathChanged(bootEvent0);

	// target path
	if (configuration->getTargetDir())
	{
		path = *configuration->getTargetDir();
	}
	targetDirPicker->SetPath(path.string());
	targetDirPicker->SetInitialDirectory(path.string());
	auto bootEvent1 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 1, path.string());
	onPathChanged(bootEvent1);

	// source token
	std::string token;
	if (configuration->getSourceToken())
		token = *configuration->getSourceToken();
	sourceTokenField->SetValue(token);
	auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
	onTokenChanged(bootEvent3);

	// target token
	if (configuration->getTargetToken())
		token = *configuration->getTargetToken();
	targetTokenField->SetValue(token);
	auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
	onTokenChanged(bootEvent4);

	// links file goes last, because it can overwrite tokens if those aren't already loaded.
	std::wstring initialPath;
	if (configuration->getLinkFile())
	{
		path = *configuration->getLinkFile();
		const auto rawFile = path.filename();
		const auto initialPath = path.parent_path();
	}
	linkFilePicker->SetPath(path.string());
	linkFilePicker->SetInitialDirectory(initialPath);
	auto bootEvent2 = wxFileDirPickerEvent(wxEVT_FILEPICKER_CHANGED, this, 2, path.string());
	onPathChanged(bootEvent2);
}

void MainFrame::initLinksFrame()
{
	auto position = wxDefaultPosition;
	if (configuration->getLinksFramePos())
		position = wxPoint(configuration->getLinksFramePos()->x, configuration->getLinksFramePos()->y);
	auto size = wxSize(600, 900);
	if (configuration->getLinksFrameSize())
		size = wxSize(configuration->getLinksFrameSize()->x, configuration->getLinksFrameSize()->y);
	const auto maximize = configuration->isLinksFrameMaximized();
	linksFrame = new LinksFrame(this, position, size, linkMapper.getVersions(), linkMapper.getActiveVersion(), configuration);

	// menubar is the thing uptop with links for actions.
	auto* saveDropDown = new wxMenu;
	saveDropDown->Append(wxID_SAVE, "Save Links [F5]\tCtrl-S");
	auto* linksDropDown = new wxMenu;
	linksDropDown->Append(wxEVT_ADD_LINK, "Add Link [F3]\tCtrl-L");
	linksDropDown->Append(wxMENU_ADD_COMMENT, "Add Comment [F4]\tCtrl-C");
	linksDropDown->Append(wxEVT_ADD_TRIANGULATION_PAIR, "Add Triangulation Point Pair [F6]");
	linksDropDown->Append(wxEVT_AUTOGENERATE_MAPPINGS, "Autogenerate mappings from triangulation pairs [F7]");
	linksDropDown->Append(wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR, "Delete Selected [Del]\tCtrl-D");
	linksDropDown->Append(wxEVT_MOVE_ACTIVE_LINK_UP, "Move Selected Up\tNum -");
	linksDropDown->Append(wxEVT_MOVE_ACTIVE_LINK_DOWN, "Move Selected Down\tNum +");
	auto* versionsDropDown = new wxMenu;
	versionsDropDown->Append(wxMENU_ADD_VERSION, "New Version");
	versionsDropDown->Append(wxMENU_COPY_VERSION, "Copy Version");
	versionsDropDown->Append(wxMENU_RENAME_VERSION, "Rename Version");
	versionsDropDown->Append(wxEVT_MOVE_ACTIVE_VERSION_LEFT, "Move Left\tNum /");
	versionsDropDown->Append(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, "Move Right\tNum *");
	versionsDropDown->Append(wxMENU_DELETE_VERSION, "Delete (Danger!)");
	auto* linksMenuBar = new wxMenuBar;
	linksMenuBar->Append(saveDropDown, "&File");
	linksMenuBar->Append(linksDropDown, "&Links/Comments");
	linksMenuBar->Append(versionsDropDown, "&Versions");
	linksFrame->SetMenuBar(linksMenuBar);

	linksFrame->Show();
	if (maximize)
		linksFrame->Maximize(true);
}

void MainFrame::initUnmappedFrame()
{
	auto position = wxDefaultPosition;
	if (configuration->getUnmappedFramePos())
		position = wxPoint(configuration->getUnmappedFramePos()->x, configuration->getUnmappedFramePos()->y);
	auto size = wxSize(300, 900);
	if (configuration->getUnmappedFrameSize())
		size = wxSize(configuration->getUnmappedFrameSize()->x, configuration->getUnmappedFrameSize()->y);
	const auto maximize = configuration->isUnmappedFrameMaximized();
	unmappedFrame = new UnmappedFrame(this, position, size, linkMapper.getActiveVersion(), configuration);

	if (configuration->isUnmappedFrameOn())
		unmappedFrame->Show();
	if (maximize)
		unmappedFrame->Maximize(true);
}

void MainFrame::initImageFrame()
{
	localizationMapper.scrapeSourceDir(*configuration->getSourceDir());
	Log(LogLevel::Info) << "Source Localizations Loaded.";
	localizationMapper.scrapeTargetDir(*configuration->getTargetDir());
	Log(LogLevel::Info) << "Target Localizations Loaded.";

	std::optional<LocalizationMapper::LocType> vic3SideloadStates;

	if (commonItems::DoesFileExist(*configuration->getSourceDir() / "definition.csv"))
	{
		auto definitions = std::make_shared<Definitions>();
		definitions->loadDefinitions(*configuration->getSourceDir(), localizationMapper, LocalizationMapper::LocType::SOURCE);
		sourceDefs = definitions;
		Log(LogLevel::Info) << "Loaded " << sourceDefs->getProvinces().size() << " source provinces.";
	}
	else if (commonItems::DoesFileExist(*configuration->getSourceDir() / "definitions.txt"))
	{
		auto definitions = std::make_shared<EU5Definitions>();
		definitions->loadDefinitions(*configuration->getSourceDir(), localizationMapper, LocalizationMapper::LocType::SOURCE);
		sourceDefs = definitions;
		Log(LogLevel::Info) << "Loaded " << sourceDefs->getProvinces().size() << " source provinces.";
	}
	else
	{
		sourceDefs = std::make_shared<Vic3Definitions>();
		sourceDefs->loadVic3Regions(*configuration->getSourceDir());
		vic3SideloadStates = LocalizationMapper::LocType::SOURCE;
		Log(LogLevel::Info) << "Loaded Vic3 source provinces.";
	}

	if (commonItems::DoesFileExist(*configuration->getTargetDir() / "definition.csv"))
	{
		auto definitions = std::make_shared<Definitions>();
		definitions->loadDefinitions(*configuration->getTargetDir(), localizationMapper, LocalizationMapper::LocType::TARGET);
		targetDefs = definitions;
		Log(LogLevel::Info) << "Loaded " << targetDefs->getProvinces().size() << " target provinces.";
	}
	else if (commonItems::DoesFileExist(*configuration->getTargetDir() / "definitions.txt"))
	{
		auto definitions = std::make_shared<EU5Definitions>();
		definitions->loadDefinitions(*configuration->getTargetDir(), localizationMapper, LocalizationMapper::LocType::TARGET);
		sourceDefs = definitions;
		Log(LogLevel::Info) << "Loaded " << targetDefs->getProvinces().size() << " target provinces.";
	}
	else
	{
		targetDefs = std::make_shared<Vic3Definitions>();
		targetDefs->loadVic3Regions(*configuration->getTargetDir());
		vic3SideloadStates = LocalizationMapper::LocType::TARGET;
		Log(LogLevel::Info) << "Loaded Vic3 target provinces.";
	}

	// Import pixels.
	wxLogNull AD; // disable warning about proprietary and thus unsupported sRGB profiles in PDX PNGs.
	sourceImg = new wxImage();
	sourceRiversImg = new wxImage();
	sourceHeightmapImg = new wxImage();
	if (commonItems::DoesFileExist(*configuration->getSourceDir() / "provinces.png"))
		sourceImg->LoadFile(configuration->getSourceDir()->string() + "/provinces.png");
	else if (commonItems::DoesFileExist(*configuration->getSourceDir() / "provinces.bmp"))
		sourceImg->LoadFile(configuration->getSourceDir()->string() + "/provinces.bmp");
	else if (commonItems::DoesFileExist(*configuration->getSourceDir() / "locations.png"))
		sourceImg->LoadFile(configuration->getSourceDir()->string() + "/locations.png");
	if (commonItems::DoesFileExist(*configuration->getSourceDir() / "rivers.png"))
		sourceRiversImg->LoadFile(configuration->getSourceDir()->string() + "/rivers.png");
	else if (commonItems::DoesFileExist(*configuration->getSourceDir() / "rivers.bmp"))
		sourceRiversImg->LoadFile(configuration->getSourceDir()->string() + "/rivers.bmp");
   	if (commonItems::DoesFileExist(*configuration->getSourceDir() / "heightmap.png"))
		sourceHeightmapImg->LoadFile(configuration->getSourceDir()->string() + "/heightmap.png");

	targetImg = new wxImage();
	targetRiversImg = new wxImage();
	targetHeightmapImg = new wxImage();
	if (commonItems::DoesFileExist(*configuration->getTargetDir() / "provinces.png"))
		targetImg->LoadFile(configuration->getTargetDir()->string() + "/provinces.png");
	else if (commonItems::DoesFileExist(*configuration->getTargetDir() / "provinces.bmp"))
		targetImg->LoadFile(configuration->getTargetDir()->string() + "/provinces.bmp");
	else if (commonItems::DoesFileExist(*configuration->getTargetDir() / "locations.png"))
		targetImg->LoadFile(configuration->getTargetDir()->string() + "/locations.png");
	if (commonItems::DoesFileExist(*configuration->getTargetDir() / "rivers.png"))
		targetRiversImg->LoadFile(configuration->getTargetDir()->string() + "/rivers.png");
	else if (commonItems::DoesFileExist(*configuration->getTargetDir() / "rivers.bmp"))
		targetRiversImg->LoadFile(configuration->getTargetDir()->string() + "/rivers.bmp");
	if (commonItems::DoesFileExist(*configuration->getTargetDir() / "heightmap.png"))
		targetHeightmapImg->LoadFile(configuration->getTargetDir()->string() + "/heightmap.png");

	mergeRivers();

	if (configuration->isSourceReversed())
		*sourceImg = sourceImg->Mirror(false);
	if (configuration->isTargetReversed())
		*targetImg = targetImg->Mirror(false);

	// Multithreading where it counts!
	auto* const pixelReader = new PixelReader(this);
	pixelReader->prepare(sourceImg, sourceDefs, configuration->isDitchAdjacencies());
	pixelReader->Create();
	pixelReader->Run();
	auto* const pixelReader2 = new PixelReader(this);
	pixelReader2->prepare(targetImg, targetDefs, configuration->isDitchAdjacencies());
	pixelReader2->Create();
	pixelReader2->Run();

	pixelReader->Wait();
	pixelReader2->Wait();

	if (vic3SideloadStates == LocalizationMapper::LocType::SOURCE)
		sourceDefs->loadLocalizations(localizationMapper, LocalizationMapper::LocType::SOURCE);
	else if (vic3SideloadStates == LocalizationMapper::LocType::TARGET)
		targetDefs->loadLocalizations(localizationMapper, LocalizationMapper::LocType::TARGET);

	if (configuration->isDitchAdjacencies())
	{
		sourceDefs->ditchAdjacencies("source_adjacencies.txt");
		targetDefs->ditchAdjacencies("target_adjacencies.txt");
	}
	else
	{
		Log(LogLevel::Info) << "Not ditching adjacencies.";
	}

	linkMapper.loadMappings(linksFile, sourceDefs, targetDefs, *configuration->getSourceToken(), *configuration->getTargetToken());
	const auto& activeLinks = linkMapper.getActiveVersion()->getLinks();
	Log(LogLevel::Info) << "Loaded " << activeLinks->size() << " active links.";

	auto position = wxDefaultPosition;
	if (configuration->getImageFramePos())
		position = wxPoint(configuration->getImageFramePos()->x, configuration->getImageFramePos()->y);
	auto size = wxSize(1200, 800);
	if (configuration->getImageFrameSize())
		size = wxSize(configuration->getImageFrameSize()->x, configuration->getImageFrameSize()->y);
	const auto maximize = configuration->isImageFrameMaximized();
	imageFrame = new ImageFrame(this,
		 position,
		 size,
		 linkMapper.getActiveVersion(),
		 sourceImg,
		 sourceHeightmapImg,
		 targetImg,
		 targetHeightmapImg,
		 sourceDefs,
		 targetDefs,
		 configuration);

	auto* menuDropDown = new wxMenu;
	menuDropDown->Append(wxID_REVERT, "Toggle Orientation");
	menuDropDown->Append(wxID_BOLD, "Toggle The Shade");
	menuDropDown->Append(ID_TOGGLE_HEIGHTMAP, "Toggle Heightmap");
	menuDropDown->Append(wxID_VIEW_SMALLICONS, "Toggle the Triangulation Mesh");
	auto* toolbarDropDown = new wxMenu;
	toolbarDropDown->Append(wxMENU_SHOW_TOOLBAR, "Show Toolbar");
	auto* unmappedDropDown = new wxMenu;
	unmappedDropDown->Append(wxMENU_SHOW_UNMAPPED, "Show Unmapped Provinces");
	auto* searchDropDown = new wxMenu;
	searchDropDown->Append(wxMENU_SHOW_SEARCH, "Show Search Window");
	auto* imageMenuBar = new wxMenuBar;
	imageMenuBar->Append(menuDropDown, "&Image");
	imageMenuBar->Append(toolbarDropDown, "&Toolbar");
	imageMenuBar->Append(unmappedDropDown, "&Unmapped");
	imageMenuBar->Append(searchDropDown, "&Search");
	imageFrame->SetMenuBar(imageMenuBar);

	imageFrame->Show();
	if (maximize)
		imageFrame->Maximize(true);
}

void MainFrame::initSearchFrame()
{
	auto position = wxDefaultPosition;
	if (configuration->getSearchFramePos())
		position = wxPoint(configuration->getSearchFramePos()->x, configuration->getSearchFramePos()->y);
	auto size = wxSize(300, 900);
	if (configuration->getSearchFrameSize())
		size = wxSize(configuration->getSearchFrameSize()->x, configuration->getSearchFrameSize()->y);
	const auto maximize = configuration->isSearchFrameMaximized();
	searchFrame = new SearchFrame(this, position, size, linkMapper.getActiveVersion(), configuration);

	if (configuration->isSearchFrameOn())
		searchFrame->Show();
	if (maximize)
		searchFrame->Maximize(true);
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

void MainFrame::onPathChanged(wxFileDirPickerEvent& evt)
{
	const auto validPath = commonItems::DoesFolderExist(std::filesystem::path(commonItems::UTF16ToUTF8(evt.GetPath().ToStdWstring())));
	const auto result = std::filesystem::path(commonItems::UTF16ToUTF8(evt.GetPath().ToStdWstring()));
	const auto green = wxColour(130, 250, 130);
	const auto red = wxColour(250, 130, 130);

	// source path
	if (evt.GetId() == 0)
	{
		if (validPath && (commonItems::DoesFileExist(result / "provinces.bmp") || commonItems::DoesFileExist(result / "provinces.png") ||
									commonItems::DoesFileExist(result / "locations.png")))
		{
			sourceDirStatus->SetBackgroundColour(green);
			sanity[0] = true;
		}
		else
		{
			sourceDirStatus->SetBackgroundColour(red);
			sanity[0] = false;
		}
		configuration->setSourceDir(result);
	}
	// target path
	else if (evt.GetId() == 1)
	{
		if (validPath && (commonItems::DoesFileExist(result / "provinces.bmp") || commonItems::DoesFileExist(result / "provinces.png") ||
									commonItems::DoesFileExist(result / "locations.png")))
		{
			targetDirStatus->SetBackgroundColour(green);
			sanity[1] = true;
		}
		else
		{
			targetDirStatus->SetBackgroundColour(red);
			sanity[1] = false;
		}
		configuration->setTargetDir(result);
	}
	// links file
	else if (evt.GetId() == 2)
	{
		if (commonItems::DoesFileExist(result))
		{
			linkFileStatus->SetBackgroundColour(green);
			linksFile = result;
			std::ifstream linksFile(result);
			if (linksFile.is_open())
				linksFileContents.assign(std::istreambuf_iterator<char>(linksFile), std::istreambuf_iterator<char>());
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
			linksFile.clear();
			linksFileContents.clear();
			sanity[2] = true;
			auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
			onTokenChanged(bootEvent3);
			auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
			onTokenChanged(bootEvent4);
		}
		else
		{
			linkFileStatus->SetBackgroundColour(red);
			linksFile.clear();
			linksFileContents.clear();
			sanity[2] = false;
			auto bootEvent3 = wxCommandEvent(wxEVT_NULL, 3);
			onTokenChanged(bootEvent3);
			auto bootEvent4 = wxCommandEvent(wxEVT_NULL, 4);
			onTokenChanged(bootEvent4);
		}
		configuration->setLinkFile(result);
	}
	configuration->save();
	applySanityToButton();
	Refresh();
}

void MainFrame::onTokenChanged(const wxCommandEvent& evt)
{
	const auto green = wxColour(130, 250, 130);
	const auto red = wxColour(250, 130, 130);

	// source token
	if (evt.GetId() == 3)
	{
		const auto input = sourceTokenField->GetValue();
		const auto rawInput = commonItems::UTF16ToUTF8(input.ToStdWstring());
		if (!rawInput.empty() && rawInput.size() >= 2 && (linksFileContents.find(rawInput) != std::string::npos || linksFileContents.empty()))
		{
			sourceTokenStatus->SetBackgroundColour(green);
			sanity[3] = true;
		}
		else
		{
			sourceTokenStatus->SetBackgroundColour(red);
			sanity[3] = false;
		}
		configuration->setSourceToken(rawInput);
	}
	// target token
	else if (evt.GetId() == 4)
	{
		const auto input = targetTokenField->GetValue();
		const auto rawInput = commonItems::UTF16ToUTF8(input.ToStdWstring());
		if (!rawInput.empty() && rawInput.size() >= 2 && (linksFileContents.find(rawInput) != std::string::npos || linksFileContents.empty()))
		{
			targetTokenStatus->SetBackgroundColour(green);
			sanity[4] = true;
		}
		else
		{
			targetTokenStatus->SetBackgroundColour(red);
			sanity[4] = false;
		}
		configuration->setTargetToken(rawInput);
	}
	configuration->save();
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
	initUnmappedFrame();
	initSearchFrame();
	Hide();
}

void MainFrame::onSaveLinks(wxCommandEvent& evt)
{
	if (!configuration->getLinkFile()->empty())
		linkMapper.exportMappings(*configuration->getLinkFile());
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

void MainFrame::onDeactivateTriangulationPair(wxCommandEvent& evt)
{
	linkMapper.deactivateTriangulationPair();
	linksFrame->deactivateTriangulationPair();
	imageFrame->deactivateTriangulationPair();
}

void MainFrame::onActivateLinkByIndex(const wxCommandEvent& evt)
{
	deactivateActiveLinkOrTriangulationPair();

	linkMapper.activateLinkByIndex(evt.GetInt());
	linksFrame->activateLinkByIndex(evt.GetInt());
	imageFrame->activateLinkByIndex(evt.GetInt());
}

void MainFrame::onActivateTriangulationPairByIndex(const wxCommandEvent& evt)
{
	deactivateActiveLinkOrTriangulationPair();

	linkMapper.activateTriangulationPairByIndex(evt.GetInt());
	linksFrame->activateTriangulationPairByIndex(evt.GetInt());
	imageFrame->activateTriangulationPairByIndex(evt.GetInt());
}

void MainFrame::deactivateActiveLinkOrTriangulationPair()
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	linkMapper.deactivateTriangulationPair();
	linksFrame->deactivateTriangulationPair();
	imageFrame->deactivateTriangulationPair();
}

void MainFrame::onRefreshActiveTriangulationPair(wxCommandEvent& evt)
{
	linksFrame->refreshActiveTriangulationPair();
}

void MainFrame::onAutogenerateMappings(const wxCommandEvent& evt)
{
	imageFrame->autogenerateMappings();
	linksFrame->redrawProvinceLinksGrid();
}

void MainFrame::onActivateLinkByID(const wxCommandEvent& evt)
{
	// This is coming from one of the imageCanvases, so we need to update everything.
	linkMapper.deactivateLink();
	imageFrame->deactivateLink();
	linksFrame->deactivateLink();
	linkMapper.activateLinkByID(evt.GetInt());
	imageFrame->activateLinkByID(evt.GetInt());
	linksFrame->activateLinkByID(evt.GetInt());
}

void MainFrame::onActivateTriangulationPairByID(const wxCommandEvent& evt)
{
	deactivateActiveLinkOrTriangulationPair();

	linkMapper.activateTriangulationPairByID(evt.GetInt());
	linksFrame->activateTriangulationPairByID(evt.GetInt());
	imageFrame->activateTriangulationPairByID(evt.GetInt());
}

void MainFrame::onToggleProvince(const wxCommandEvent& evt)
{
	// Two things can happen. We're either:
	// 1. toggling a province within our active link
	// 2. toggling a province without active link, thus creating one.
	// In the second case we need to update quite a lot of things.

	bool sourceImage = true;
	auto ID = evt.GetString().ToStdString();
	if (ID.starts_with('-'))
	{
		ID = ID.substr(1, ID.length());
		sourceImage = false;
	}

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

	// it is unclear what happened to the province. It may be mapped or unmapped so we poke.
	const auto result = linkMapper.isProvinceMapped(ID, sourceImage);
	if (result == Mapping::MAPPED)
		unmappedFrame->removeProvince(ID, sourceImage);
	else if (result == Mapping::UNMAPPED)
		unmappedFrame->addProvince(ID, sourceImage);
	// else something went wrong and better don't touch.
}

void MainFrame::onCenterMap(const wxCommandEvent& evt)
{
	imageFrame->centerMap(evt.GetInt());
}

void MainFrame::onCenterProvince(const wxCommandEvent& evt)
{
	ImageTabSelector selector;
	if (evt.GetId() == 0)
		selector = ImageTabSelector::SOURCE;
	else
		selector = ImageTabSelector::TARGET;
	imageFrame->centerProvince(selector, evt.GetString().ToStdString());
}

void MainFrame::onCenterMapToTriangulationPair(const wxCommandEvent& evt)
{
	imageFrame->centerMapToTriangulationPair(evt.GetInt());
}

void MainFrame::onAddComment(const wxCommandEvent& evt)
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
	imageFrame->deleteActiveLink(); // Images first so it knows which provinces to recolor.
	linkMapper.deleteActiveLink();
	linksFrame->deactivateLink();
	unmappedFrame->refreshList();
	searchFrame->refreshList();
}

void MainFrame::onDeleteActiveTriangulationPair(wxCommandEvent& evt)
{
	// We don't need an ID since this works only on active pair.
	linkMapper.deleteActiveTriangulationPair();
	linksFrame->deactivateTriangulationPair();
	imageFrame->deleteActiveTriangulationPair();
}

void MainFrame::onDeleteActiveLinkOrTriangulationPair(wxCommandEvent& evt)
{
	const auto& version = linkMapper.getActiveVersion();
	if (!version)
		return;

	if (version->getActiveTriangulationPair())
	{
		onDeleteActiveTriangulationPair(evt);
		return;
	}

	onDeleteActiveLink(evt);
}

void MainFrame::onLinksAddLink(wxCommandEvent& evt)
{
	// This one creates a link without any provinces.
	const auto newLinkID = linkMapper.addRawLink();
	if (newLinkID)
	{
		linksFrame->deactivateTriangulationPair();
		linksFrame->createLink(*newLinkID);
		imageFrame->deactivateLink();
		imageFrame->deactivateTriangulationPair();
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::onLinksAddComment(wxCommandEvent& evt)
{
	// This one creates a blank comment at unspecified location.
	const auto newLinkID = linkMapper.addRawComment();
	if (newLinkID)
	{
		linksFrame->deactivateTriangulationPair();
		linksFrame->createLink(*newLinkID);
		imageFrame->deactivateLink();
		imageFrame->deactivateTriangulationPair();
		imageFrame->activateLinkByID(*newLinkID);
	}
}

void MainFrame::onLinksAddTriangulationPair(wxCommandEvent& evt)
{
	// This one creates a new triangulation point pair.
	const auto newPairID = linkMapper.addRawTriangulationPair();
	if (newPairID)
	{
		linksFrame->deactivateLink();
		linksFrame->createTriangulationPair(*newPairID);
		imageFrame->deactivateLink();
		imageFrame->deactivateTriangulationPair();
		imageFrame->activateTriangulationPairByID(*newPairID);
	}
}

void MainFrame::mergeRivers() const
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

void MainFrame::mergeRiverData(unsigned char* imgData, const unsigned char* riverData, const int size)
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
	unmappedFrame->setVersion(newVersion);
	searchFrame->setVersion(newVersion);
}

void MainFrame::onVersionsCopyVersion(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& newVersion = linkMapper.copyVersion();
	linksFrame->addVersion(newVersion);
	imageFrame->setVersion(newVersion);
	unmappedFrame->setVersion(newVersion);
	searchFrame->setVersion(newVersion);
}

void MainFrame::onVersionsDeleteVersion(wxCommandEvent& evt)
{
	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& activeVersion = linkMapper.deleteVersion();
	linksFrame->deleteActiveAndSwapToVersion(activeVersion);
	imageFrame->setVersion(activeVersion);
	unmappedFrame->setVersion(activeVersion);
	searchFrame->setVersion(activeVersion);
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

void MainFrame::onRenameVersion(const wxCommandEvent& evt)
{
	const auto versionName = evt.GetString().ToStdString();

	linkMapper.updateActiveVersionName(versionName);
	linksFrame->updateActiveVersionName(versionName);
}

void MainFrame::onChangeTab(const wxBookCtrlEvent& event)
{
	// linksTab has changed tab to another set. We need to update everything.

	linkMapper.deactivateLink();
	linksFrame->deactivateLink();
	imageFrame->deactivateLink();

	const auto& activeVersion = linkMapper.activateVersionByIndex(event.GetSelection());
	linksFrame->setVersion(activeVersion);
	imageFrame->setVersion(activeVersion);
	unmappedFrame->setVersion(activeVersion);
	searchFrame->setVersion(activeVersion);
}

void MainFrame::onLinksMoveUp(wxCommandEvent& evt)
{
	linkMapper.moveActiveLinkUp();
	linksFrame->moveActiveLinkUp();
}

void MainFrame::onLinksMoveDown(wxCommandEvent& evt)
{
	linkMapper.moveActiveLinkDown();
	linksFrame->moveActiveLinkDown();
}

void MainFrame::onLinksMoveVersionLeft(wxCommandEvent& evt)
{
	linkMapper.moveActiveVersionLeft();
	linksFrame->moveActiveVersionLeft();
}

void MainFrame::onLinksMoveVersionRight(wxCommandEvent& evt)
{
	linkMapper.moveActiveVersionRight();
	linksFrame->moveActiveVersionRight();
}

void MainFrame::onShowToolbar(wxCommandEvent& evt)
{
	imageFrame->showToolbar();
}

void MainFrame::onShowUnmapped(wxCommandEvent& evt)
{
	configuration->setUnmappedFrameOn(true);
	configuration->save();
	unmappedFrame->Show();
}

void MainFrame::onShowSearch(wxCommandEvent& evt)
{
	configuration->setSearchFrameOn(true);
	configuration->save();
	searchFrame->Show();
}
