#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
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
class LinkMapping;
class ProvinceMappingsGrid final: public wxGrid
{
  public:
	ProvinceMappingsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);

	void redraw();

  private:
	void onGridMotion(wxMouseEvent& event);

	std::shared_ptr<LinkMappingVersion> version;

	void focusOnActiveRow();
	static std::string linkToString(const std::shared_ptr<LinkMapping>& link);
};