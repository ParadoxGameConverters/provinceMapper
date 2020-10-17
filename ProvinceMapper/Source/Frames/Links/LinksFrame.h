#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "LinksTab.h"
#include <map>
#include <wx/notebook.h>

class LinkMappingVersion;
class LinksFrame: public wxFrame
{
  public:
	LinksFrame(wxWindow* parent, const std::vector<std::shared_ptr<LinkMappingVersion>>& versions, const std::shared_ptr<LinkMappingVersion>& activeVersion);

	void deactivateLink() const;
	void activateLinkByID(int ID) const;
	void refreshActiveLink() const;

  private:
	wxNotebook* notebook = nullptr;

	void onResize(wxSizeEvent& evt);
	void onClose(wxCloseEvent& event);

	std::map<int, LinksTab*> pages;
	std::pair<int, LinksTab*> activePage;
	std::map<std::string, int> versionToPage;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
