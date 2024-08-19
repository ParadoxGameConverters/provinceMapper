#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "GridBase.h"
#include "LinkMapper/LinkMappingVersion.h"
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_REDRAW_LINKS_GRID, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class ProvinceMappingsGrid final: public GridBase
{
  public:
	ProvinceMappingsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);

	void redraw();
	void leftUp(const wxGridEvent& event) override;
	void rightUp(wxGridEvent& event) override;

	void deactivateLink();
	void activateLinkByID(int theID);
	void activateLinkByIndex(int index);

	void activateLinkRowColor(int row);
	void restoreLinkRowColor(int row);

	void createLink(int linkID);

	void stageAddComment();

	[[nodiscard]] const std::shared_ptr<LinkBase> getActiveLink() override { return version->getActiveLink(); }

  private:
	void onUpdateComment(const wxCommandEvent& event);
};