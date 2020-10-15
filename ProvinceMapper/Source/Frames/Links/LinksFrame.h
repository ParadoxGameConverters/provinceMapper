#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wx/notebook.h>

#include "LinksTab.h"

class LinkMappingVersion;
class LinksFrame: public wxFrame
{
  public:
	LinksFrame(wxWindow* parent, std::vector<std::shared_ptr<LinkMappingVersion>> theVersions, std::shared_ptr<LinkMappingVersion> theActiveVersion);

  private:
	wxNotebook* notebook = nullptr;

	void onResize(wxSizeEvent& evt);
	
	std::vector<std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
	std::map<int, LinksTab*> pages;
	std::pair<int, LinksTab*> activePage;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
