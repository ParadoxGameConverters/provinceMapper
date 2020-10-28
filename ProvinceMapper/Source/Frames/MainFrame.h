#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Configuration/Configuration.h"
#include "Definitions/Definitions.h"
#include "LinkMapper/LinkMapper.h"
#include "Localization/LocalizationMapper.h"
#include <array>
#include <wx/splitter.h>

wxDECLARE_EVENT(wxMENU_ADD_COMMENT, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_ADD_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_COPY_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_DELETE_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_RENAME_VERSION, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_SHOW_TOOLBAR, wxCommandEvent);
wxDECLARE_EVENT(wxMENU_SHOW_UNMAPPED, wxCommandEvent);

class UnmappedFrame;
class wxBookCtrlEvent;
class wxFilePickerCtrl;
class wxDirPickerCtrl;
class wxFileDirPickerEvent;
enum class ImageTabSelector;
class ImageFrame;
class LinksFrame;
class MainFrame: public wxFrame
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
	void mergeRivers() const;
	static void mergeRiverData(unsigned char* imgData, unsigned char* riverData, int size);
	static bool isRiverMask(unsigned char r, unsigned char g, unsigned char b);

	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);
	void onSupportUs(wxCommandEvent& event);
	void onPathChanged(wxFileDirPickerEvent& evt);
	void onTokenChanged(wxCommandEvent& evt);
	void onStartButton(wxCommandEvent& evt);
	void onSaveLinks(wxCommandEvent& evt);
	void onDeactivateLink(wxCommandEvent& evt);
	void onActivateLinkByIndex(wxCommandEvent& evt);
	void onActivateLinkByID(wxCommandEvent& evt);
	void onToggleProvince(wxCommandEvent& evt);
	void onCenterMap(wxCommandEvent& evt);
	void onCenterProvince(wxCommandEvent& evt);
	void onAddComment(wxCommandEvent& evt);
	void onDeleteActiveLink(wxCommandEvent& evt);
	void onLinksAddLink(wxCommandEvent& evt);
	void onLinksAddComment(wxCommandEvent& evt);
	void onVersionsAddVersion(wxCommandEvent& evt);
	void onVersionsCopyVersion(wxCommandEvent& evt);
	void onVersionsDeleteVersion(wxCommandEvent& evt);
	void onVersionsRenameVersion(wxCommandEvent& evt);
	void onRenameVersion(wxCommandEvent& evt);
	void onChangeTab(wxBookCtrlEvent& event);
	void onLinksMoveUp(wxCommandEvent& evt);
	void onLinksMoveDown(wxCommandEvent& evt);
	void onLinksMoveVersionLeft(wxCommandEvent& evt);
	void onLinksMoveVersionRight(wxCommandEvent& evt);
	void onShowToolbar(wxCommandEvent& evt);
	void onShowUnmapped(wxCommandEvent& evt);

	LinksFrame* linksFrame = nullptr;
	UnmappedFrame* unmappedFrame = nullptr;
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

	std::shared_ptr<Definitions> sourceDefs;
	std::shared_ptr<Definitions> targetDefs;
	LinkMapper linkMapper;
	std::shared_ptr<Configuration> configuration;
	LocalizationMapper localizationMapper;

	std::string linksFileString;

	// source/target pickers, links, source/target tokens
	std::array<bool, 5> sanity = {false, false, false, false, false};
};
