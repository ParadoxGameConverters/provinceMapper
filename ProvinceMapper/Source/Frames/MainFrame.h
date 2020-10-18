#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Configuration/Configuration.h"
#include "Definitions/Definitions.h"
#include "LinkMapper/LinkMapper.h"
#include <array>
#include <wx/splitter.h>

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
	void onAddComment(wxCommandEvent& evt);
	void onDeleteActiveLink(wxCommandEvent& evt);

	void readPixels(ImageTabSelector selector, const wxImage& img);
	static bool isSameColorAtCoords(int ax, int ay, int bx, int by, const wxImage& img);

	LinksFrame* linksFrame = nullptr;
	wxFlexGridSizer* sizer = nullptr;

	ImageFrame* imageFrame = nullptr;
	wxImage* sourceImg = nullptr;
	wxImage* targetImg = nullptr;

	wxDirPickerCtrl* sourceDirPicker = nullptr;
	wxDirPickerCtrl* targetDirPicker = nullptr;
	wxFilePickerCtrl* linkFilePicker = nullptr;
	wxTextCtrl* sourceTokenField = nullptr;
	wxTextCtrl* targetTokenField = nullptr;

	wxWindow* sourceDirStatus = nullptr;
	wxWindow* targetDirStatus = nullptr;
	wxWindow* linkFileStatus = nullptr;
	wxWindow* sourceTokenStatus = nullptr;
	wxWindow* targetTokenStatus = nullptr;

	wxButton* startButton = nullptr;

	std::shared_ptr<Definitions> sourceDefs;
	std::shared_ptr<Definitions> targetDefs;
	LinkMapper linkMapper;
	Configuration configuration;

	std::string linksFileString;

	std::array<bool, 5> sanity = {false, false, false, false, false}; // source/target pickers, links, source/target tokens
};
