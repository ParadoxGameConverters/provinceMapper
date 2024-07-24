#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>
#include "GridBase.h"



wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class TriangulationPointPair;
class TriangulationPairsGrid final: public GridBase
{
  public:
	TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void focusOnActiveRow();
	void redraw();
	void stageAddComment();

	void rightUp(wxGridEvent& event);

	void createTriangulationPair(int pairID);

  private:
	void triangulationPairsGridLeftUp(const wxGridEvent& event);

	std::shared_ptr<TriangulationPointPair> activeTriangulationPair;
};