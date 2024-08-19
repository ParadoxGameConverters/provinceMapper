#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "GridBase.h"
#include "LinkMapper/LinkMappingVersion.h"

#include <optional>
#include <wx/grid.h>



wxDECLARE_EVENT(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_REFRESH_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_AUTOGENERATE_MAPPINGS, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class TriangulationPointPair;
class TriangulationPairsGrid final: public GridBase
{
  public:
	TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void redraw();
	void stageAddComment();

	void leftUp(const wxGridEvent& event);
	void rightUp(wxGridEvent& event);

	void activateLinkRowColor(int row);
	void restoreLinkRowColor(int row);

	void deactivateTriangulationPair();
	void activatePairByIndex(const int index);
	void createTriangulationPair(int pairID);

	[[nodiscard]] const std::shared_ptr<LinkBase> getActiveLink() override { return version->getActiveTriangulationPair(); }

  private:

	void onUpdateComment(const wxCommandEvent& event);
};