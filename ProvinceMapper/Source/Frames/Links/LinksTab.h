#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/grid.h>
#include <wx/notebook.h>

class LinkMappingVersion;
class LinksTab: public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID);
	void redrawGrid() const;

  private:
	wxGrid* theGrid = nullptr;
	int ID = 0;

	std::shared_ptr<LinkMappingVersion> version;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
