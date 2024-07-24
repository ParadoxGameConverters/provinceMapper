#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>



wxDECLARE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class TriangulationPointPair;
class TriangulationPairsGrid final: public wxGrid
{
  public:
	TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	std::shared_ptr<TriangulationPointPair> activeTriangulationPair;
	void focusOnActiveRow();
	void redraw();

  private:
	std::optional<int> activeTriangulationPointRow;
	std::shared_ptr<LinkMappingVersion> version;

	void onGridMotion(wxMouseEvent& event);
	void triangulationPairsGridLeftUp(const wxGridEvent& event);
};