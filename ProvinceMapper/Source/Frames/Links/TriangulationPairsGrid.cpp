#include "TriangulationPairsGrid.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);


TriangulationPairsGrid::TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion) : wxGrid(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND), version(theVersion) {
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



void TriangulationPairsGrid::triangulationPairsGridLeftUp(const wxGridEvent& event) // TODO: move this to TriangulationPairsTab
{
	// Left Up means:
	// 1. We want to mark a nonworking row as working row
	// 2. We are AGAIN clicking on a working row to center the map
	// 3. We're AGAIN clicking on a comment to change it.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(version->getTriangulationPointPairs()->size()))
	{
		// Case 3: This is a comment.
		if (version->getTriangulationPointPairs()->at(row)->getComment())
		{
			// and we're altering it.
			if (activeTriangulationPointRow && *activeTriangulationPointRow == row)
			{
				// spawn a dialog to change the name.
				// TODO: DialogComment is for links. Implement an equivalent for triangulation pairs.
				auto* dialog = new DialogComment(this, "Edit Comment", *version->getTriangulationPointPairs()->at(row)->getComment(), row);
				dialog->ShowModal();
				return;
			}
		}

		// Case 2: if we already clicked here, center the map.
		if (activeTriangulationPointRow && *activeTriangulationPointRow == row)
		{
			auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR);
			centerEvt->SetInt(activeTriangulationPair->getID());
			eventListener->QueueEvent(centerEvt->Clone());
			return;
		}

		// Case 1: Selecting a new row.
		if (activeTriangulationPointRow)
			restoreLinkRowColor(*activeTriangulationPointRow);

		auto* evt = new wxCommandEvent(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());

		lastClickedTriangulationPairRow = row;
	}
}

void TriangulationPairsGrid::redraw()
 {
	 auto rowCounter = 0;
	 triangulationPointGrid->BeginBatch();
	 triangulationPointGrid->DeleteRows(0, theGrid->GetNumberRows());

	 for (const auto& pair: *version->getTriangulationPointPairs())
	 {
		 auto bgColor = wxColour(240, 240, 240);
		 std::string name;
		 name = triangulationPairToString(pair);
		 if (activeTriangulationPair && *pair == *activeTriangulationPair)
		 {
			 bgColor = wxColour(150, 250, 150); // bright green for selected triangulation pairs
			 activeTriangulationPointRow = rowCounter;
		 }
		 triangulationPointGrid->AppendRows(1, false);
		 triangulationPointGrid->SetRowSize(rowCounter, 20);
		 triangulationPointGrid->SetCellValue(rowCounter, 0, name);
		 triangulationPointGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		 triangulationPointGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		 rowCounter++;
	 }
	 triangulationPointGrid->AutoSizeColumn(0, false);
	 triangulationPointGrid->EndBatch();
	 if (activeTriangulationPointRow)
		 focusOnActiveTriangulationPairRow();
	 GetParent()->Layout();
	 triangulationPointGrid->ForceRefresh();
 }