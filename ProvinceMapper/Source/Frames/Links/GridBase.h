#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);


class LinkMappingVersion;
class LinkBase;
class GridBase : public wxGrid 
{
  public:
	GridBase(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);

	std::shared_ptr<LinkBase> activeLink;

  private:
	void onGridMotion(wxMouseEvent& event);

  protected:
	void focusOnActiveRow();
	void moveActiveLinkUp();
	void moveActiveLinkDown();

	std::shared_ptr<LinkMappingVersion> version;

	std::optional<int> activeRow;
	int lastClickedRow = 0;

	wxEvtHandler* eventListener = nullptr;
};
