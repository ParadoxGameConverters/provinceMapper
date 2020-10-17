#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>

wxDECLARE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);

class LinkMappingVersion;
class LinkMapping;
class LinksTab: public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID);
	void redrawGrid();

	void deactivateLink();
	void activateLinkByID(int theID);
	void refreshActiveLink();

  private:
	int ID = 0;

	wxGrid* theGrid = nullptr;
	std::optional<int> activeRow;
	std::shared_ptr<LinkMapping> activeLink;
	std::shared_ptr<LinkMappingVersion> version;

	void onCellSelect(wxGridEvent& event);
	void focusOnActiveRow();
	static std::string linkToString(const std::shared_ptr<LinkMapping>& link);

  protected:
	wxEvtHandler* eventListener = nullptr;
};
