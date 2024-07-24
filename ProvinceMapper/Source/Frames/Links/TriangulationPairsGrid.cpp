#include "TriangulationPairsGrid.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
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
		auto bgColor = pair->getBaseRowColour();
		if (activeTriangulationPair && *pair == *activeTriangulationPair)
		{
			bgColor = pair->getActiveRowColour(); // bright green for selected triangulation pairs
			activeRow = rowCounter;
		}
		AppendRows(1, false);
		SetRowSize(rowCounter, 20);
		SetCellValue(rowCounter, 0, pair->toRowString());
		SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		SetCellBackgroundColour(rowCounter, 0, bgColor);
		rowCounter++;
	}
	AutoSizeColumn(0, false);
	EndBatch();
	if (activeRow)
		focusOnActiveRow();
	GetParent()->Layout();
	triangulationPointGrid->ForceRefresh();
}


void TriangulationPairsGrid::focusOnActiveRow()
{
	const auto cellCoords = CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
	Scroll(offset);														  // and shoo.
	ForceRefresh();
}

void TriangulationPairsGrid::stageAddComment()
{
	auto* dialog = new DialogComment(this, "Add Comment", lastClickedRow);
	dialog->ShowModal();
}

void TriangulationPairsGrid::rightUp(wxGridEvent& event)
{
	const wxCommandEvent* evt;

	// Right up means deselect active link, which is serious stuff.
	// If our active link is dry, we're not deselecting it, we're deleting it.
	evt = new wxCommandEvent(wxEVT_DEACTIVATE_TRIANGULATION_PAIR);

	eventListener->QueueEvent(evt->Clone());
	event.Skip();
}


void TriangulationPairsGrid::createTriangulationPair(int pairID)
{
	// We could just redraw the entire grid but that flickers. This is more complicated but cleaner on the eyes.

	// Where is this new row?
	auto rowCounter = 0;
	for (const auto& pair: *version->getTriangulationPointPairs())
	{
		if (pair->getID() == pairID)
		{
			InsertRows(rowCounter, 1, false);
			SetCellValue(rowCounter, 0, pair->toRowString());

			activateTriangulationPairRowColor(rowCounter);
			activeTriangulationPair = pair;
			// If we have an active link, restore its color.
			if (activeRow)
				restoreTriangulationPairRowColor(*activeRow + 1); // We have a link inserted so we need to fix the following one.
			activeTriangulationPointRow = rowCounter;
			lastClickedTriangulationPairRow = rowCounter;
			// let's insert it.
			triangulationPointGrid->SetColMinimalWidth(0, 600);
			triangulationPointGrid->ForceRefresh();
			break;
		}
		++rowCounter;
	}
}