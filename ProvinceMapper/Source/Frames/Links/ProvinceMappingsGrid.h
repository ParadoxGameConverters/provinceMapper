#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>
#include "GridBase.h"


wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent); // TODO: move to linkstab
wxDECLARE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class ProvinceMappingsGrid final: public GridBase
{
  public:
	ProvinceMappingsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);

	void redraw();
	void leftUp(const wxGridEvent& event);
	void rightUp(wxGridEvent& event);

	void deactivateLink();
	void activateLinkByID(const int theID);
	void activateLinkByIndex(const int index);

	void activateLinkRowColor(int row);
	void restoreLinkRowColor(int row);

	void createLink(const int linkID);

	void stageAddComment();

  private:
	void onUpdateComment(const wxCommandEvent& event);
};