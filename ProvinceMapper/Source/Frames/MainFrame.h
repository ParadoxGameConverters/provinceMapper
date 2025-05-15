#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Configuration/Configuration.h"
#include "Definitions/DefinitionsInterface.h"
#include "LinkMapper/LinkMapper.h"
#include "Localization/LocalizationMapper.h"
#include <array>
#include <wx/splitter.h>

class SearchFrame;
wxDECLARE_EVENT(wxMENU_ADD_COMMENT, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_ADD_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_COPY_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_DELETE_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_RENAME_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_SHOW_TOOLBAR, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_SHOW_UNMAPPED, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_SHOW_SEARCH, wxCommandEvent);

class UnmappedFrame;
class wxBookCtrlEvent;
class wxFilePickerCtrl;
class wxDirPickerCtrl;
class wxFileDirPickerEvent;
enum class ImageTabSelector;
class ImageFrame;
class LinksFrame;
class MainFrame final : public wxFrame
{
  public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	void initFrame();

  private:
	void populateFrame();
	void applySanityToButton();
	void initImageFrame();
	void initLinksFrame();
	void initUnmappedFrame();
	void initSearchFrame();
	void mergeRivers() const;
	static void mergeRiverData(unsigned char* imgData, const unsigned char* riverData, const int size);
	static bool isRiverMask(unsigned char r, unsigned char g, unsigned char b);

	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);
	void onSupportUs(wxCommandEvent& event);
	void onPathChanged(wxFileDirPickerEvent& evt);
	void onTokenChanged(const wxCommandEvent& evt);
	void onStartButton(wxCommandEvent& evt);
	void onSaveLinks(wxCommandEvent& evt);
	void onDeactivateLink(wxCommandEvent& evt);
	void onDeactivateTriangulationPair(wxCommandEvent& evt);
	void onActivateLinkByIndex(const wxCommandEvent& evt);
	void onActivateTriangulationPairByIndex(const wxCommandEvent& evt);
	void onActivateLinkByID(const wxCommandEvent& evt);
	void onActivateTriangulationPairByID(const wxCommandEvent& evt);
	void onToggleProvince(const wxCommandEvent& evt);
	void onCenterMap(const wxCommandEvent& evt);
	void onCenterProvince(const wxCommandEvent& evt);
	void onCenterMapToTriangulationPair(const wxCommandEvent& evt);
	void onAddComment(const wxCommandEvent& evt);
	void onDeleteActiveLink(wxCommandEvent& evt);
	void onDeleteActiveTriangulationPair(wxCommandEvent& evt);
	void onDeleteActiveLinkOrTriangulationPair(wxCommandEvent& evt);
	void onLinksAddLink(wxCommandEvent& evt);
	void onLinksAddComment(wxCommandEvent& evt);
	void onLinksAddTriangulationPair(wxCommandEvent& evt);
	void onVersionsAddVersion(wxCommandEvent& evt);
	void onVersionsCopyVersion(wxCommandEvent& evt);
	void onVersionsDeleteVersion(wxCommandEvent& evt);
	void onVersionsRenameVersion(wxCommandEvent& evt);
	void onRenameVersion(const wxCommandEvent& evt);
	void onChangeTab(const wxBookCtrlEvent& event);
	void onLinksMoveUp(wxCommandEvent& evt);
	void onLinksMoveDown(wxCommandEvent& evt);
	void onLinksMoveVersionLeft(wxCommandEvent& evt);
	void onLinksMoveVersionRight(wxCommandEvent& evt);
	void onShowToolbar(wxCommandEvent& evt);
	void onShowUnmapped(wxCommandEvent& evt);
	void onShowSearch(wxCommandEvent& evt);
	void deactivateActiveLinkOrTriangulationPair();
	void onRefreshActiveTriangulationPair(wxCommandEvent& evt);
	void onAutogenerateMappings(const wxCommandEvent& evt);

	LinksFrame* linksFrame = nullptr;
	UnmappedFrame* unmappedFrame = nullptr;
	SearchFrame* searchFrame = nullptr;
	wxFlexGridSizer* sizer = nullptr;

	ImageFrame* imageFrame = nullptr;
	wxImage* sourceImg = nullptr;
	wxImage* targetImg = nullptr;
	wxImage* sourceRiversImg = nullptr;
	wxImage* targetRiversImg = nullptr;

	wxDirPickerCtrl* sourceDirPicker = nullptr;
	wxDirPickerCtrl* targetDirPicker = nullptr;
	wxFilePickerCtrl* linkFilePicker = nullptr;
	wxTextCtrl* sourceTokenField = nullptr;
	wxTextCtrl* targetTokenField = nullptr;
	wxCheckBox* reverseSourceCheck = nullptr;
	wxCheckBox* reverseTargetCheck = nullptr;

	wxWindow* sourceDirStatus = nullptr;
	wxWindow* targetDirStatus = nullptr;
	wxWindow* linkFileStatus = nullptr;
	wxWindow* sourceTokenStatus = nullptr;
	wxWindow* targetTokenStatus = nullptr;

	wxButton* startButton = nullptr;

	std::shared_ptr<DefinitionsInterface> sourceDefs;
	std::shared_ptr<DefinitionsInterface> targetDefs;
	LinkMapper linkMapper;
	std::shared_ptr<Configuration> configuration;
	LocalizationMapper localizationMapper;

	std::string linksFileString;

	// source/target pickers, links, source/target tokens
	std::array<bool, 5> sanity = {false, false, false, false, false};
};
