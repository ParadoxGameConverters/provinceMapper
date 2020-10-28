#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wx/notebook.h>

class Configuration;
class LinkMappingVersion;
class UnmappedFrame: public wxFrame
{
  public:
	UnmappedFrame(wxWindow* parent,
		 const wxPoint& position,
		 const wxSize& size,
		 const std::shared_ptr<LinkMappingVersion>& activeVersion,
		 std::shared_ptr<Configuration> theConfiguration);

	void setVersion(const std::shared_ptr<LinkMappingVersion>& version);

  private:
	wxNotebook* notebook = nullptr;

	void onResize(wxSizeEvent& evt);
	void onClose(wxCloseEvent& event);
	void onMove(wxMoveEvent& event);

	std::map<int, LinksTab*> pages; // version ID, page it's on.
	LinksTab* activePage = nullptr;
	std::vector<int> versionIDs; // this mirrors our pages so first page holds a version with first ID in this vector.

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
