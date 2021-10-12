#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "LinksTab.h"
#include <map>
#include <wx/notebook.h>

class LinkMappingVersion;
class Configuration;
class LinksFrame final : public wxFrame
{
  public:
	LinksFrame(wxWindow* parent,
		 const wxPoint& position,
		 const wxSize& size,
		 const std::vector<std::shared_ptr<LinkMappingVersion>>& versions,
		 const std::shared_ptr<LinkMappingVersion>& activeVersion,
		 std::shared_ptr<Configuration> theConfiguration);

	void deactivateLink() const;
	void activateLinkByID(int ID) const;
	void activateLinkByIndex(int index) const;
	void refreshActiveLink() const;
	void createLink(int linkID) const;
	void addVersion(const std::shared_ptr<LinkMappingVersion>& version);
	void deleteActiveAndSwapToVersion(const std::shared_ptr<LinkMappingVersion>& version);
	void setVersion(const std::shared_ptr<LinkMappingVersion>& version);
	void updateActiveVersionName(const std::string& theName) const;
	void moveActiveLinkUp() const;
	void moveActiveLinkDown() const;
	void moveActiveVersionLeft();
	void moveActiveVersionRight();

  private:
	wxNotebook* notebook = nullptr;

	void onResize(wxSizeEvent& event);
	void onClose(wxCloseEvent& event);
	void onKeyDown(wxKeyEvent& event);
	void onMove(wxMoveEvent& event);

	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

	std::map<int, LinksTab*> pages; // version ID, page it's on.
	LinksTab* activePage = nullptr;
	std::vector<int> versionIDs; // this mirrors our pages so first page holds a version with first ID in this vector.

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
