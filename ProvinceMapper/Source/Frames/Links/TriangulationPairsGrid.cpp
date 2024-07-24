#include "TriangulationPairsGrid.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);


TriangulationPairsGrid::TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion) : GridBase(parent, theVersion)
{
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
			if (activeRow && *activeRow == row)
			{
				// spawn a dialog to change the name.
				// TODO: DialogComment is for links. Implement an equivalent for triangulation pairs.
				auto* dialog = new DialogComment(this, "Edit Comment", *version->getTriangulationPointPairs()->at(row)->getComment(), row);
				dialog->ShowModal();
				return;
			}
		}

		// Case 2: if we already clicked here, center the map.
		if (activeRow && *activeRow == row)
		{
			auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR);
			centerEvt->SetInt(activeTriangulationPair->getID());
			eventListener->QueueEvent(centerEvt->Clone());
			return;
		}

		// Case 1: Selecting a new row.
		if (activeRow)
			restoreLinkRowColor(*activeRow);

		auto* evt = new wxCommandEvent(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());

		lastClickedTriangulationPairRow = row;
	}
}

void TriangulationPairsGrid::redraw()
{
	auto rowCounter = 0;
	BeginBatch();
	DeleteRows(0, GetNumberRows());

	for (const auto& pair: *version->getTriangulationPointPairs())
	{
		auto bgColor = wxColour(240, 240, 240);
		std::string name;
		name = triangulationPairToString(pair);
		if (activeTriangulationPair && *pair == *activeTriangulationPair)
		{
			bgColor = wxColour(150, 250, 150); // bright green for selected triangulation pairs
			activeRow = rowCounter;
		}
		AppendRows(1, false);
		SetRowSize(rowCounter, 20);
		SetCellValue(rowCounter, 0, name);
		SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		SetCellBackgroundColour(rowCounter, 0, bgColor);
		rowCounter++;
	}
	triangulationPointGrid->AutoSizeColumn(0, false);
	triangulationPointGrid->EndBatch();
	if (activeRow)
		focusOnActiveRow();
	GetParent()->Layout();
	triangulationPointGrid->ForceRefresh();
}


void TriangulationPairsGrid::focusOnActiveRow()
{
	const auto cellCoords = triangulationPointGrid->CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = triangulationPointGrid->GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
	triangulationPointGrid->Scroll(offset);														  // and shoo.
	triangulationPointGrid->ForceRefresh();
}