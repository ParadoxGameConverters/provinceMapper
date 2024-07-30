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
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);


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

	void leftUp(const wxGridEvent& event);
	void rightUp(wxGridEvent& event);

	void activateLinkRowColor(int row);
	void restoreLinkRowColor(int row);

	void createTriangulationPair(int pairID);

  private:
	std::shared_ptr<TriangulationPointPair> activeTriangulationPair;

	void onUpdateComment(const wxCommandEvent& event);
};