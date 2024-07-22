#include "TriangulationPairsGrid.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);


TriangulationPairsGrid::TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion) : wxGrid(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND) {
	CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	EnableEditing(false);
	HideCellEditControl();
	HideRowLabels();
	HideColLabels();
	SetScrollRate(0, 10);
	SetColMinimalAcceptableWidth(600);
	GetGridWindow()->Bind(wxEVT_MOTION, &TriangulationPairsGrid::onGridMotion, this);
	SetColMinimalWidth(0, 600);
}


void TriangulationPairsGrid::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}