#include "TriangulationPairsGrid.h"

#include "LinkMapper/LinkMappingVersion.h"
#include <Log.h>
#include "Provinces/Province.h"
#include "TriangulationPairDialogComment.h"
#include <utility>



wxDEFINE_EVENT(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_UP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_DOWN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_REFRESH_ACTIVE_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_AUTOGENERATE_MAPPINGS, wxCommandEvent);


TriangulationPairsGrid::TriangulationPairsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion): GridBase(parent, std::move(theVersion))
{
	Bind(wxEVT_UPDATE_TRIANGULATION_PAIR_COMMENT, &TriangulationPairsGrid::onUpdateComment, this);
}


void TriangulationPairsGrid::leftUp(const wxGridEvent& event)
{
	// Left Up means:
	// 1. We want to mark a nonworking row as working row
	// 2. We are AGAIN clicking on a working row to center the map
	// 3. We're AGAIN clicking on a comment to change it.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(version->getTriangulationPairs()->size()))
	{
		// Case 3: This is a comment.
		if (version->getTriangulationPairs()->at(row)->getComment())
		{
			// and we're altering it.
			if (activeRow && *activeRow == row)
			{
				// spawn a dialog to change the name.
				auto* dialog = new TriangulationPairDialogComment(this, "Edit Comment", *version->getTriangulationPairs()->at(row)->getComment(), row);
				dialog->ShowModal();
				return;
			}
		}

		// Case 2: if we already clicked here, center the map.
		if (activeRow && *activeRow == row)
		{
			auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP_TO_TRIANGULATION_PAIR);
			centerEvt->SetInt(getActiveLink()->getID());
			eventListener->QueueEvent(centerEvt->Clone());
			return;
		}

		// Case 1: Selecting a new row.
		if (activeRow)
			restoreLinkRowColor(*activeRow);

		auto* evt = new wxCommandEvent(wxEVT_SELECT_TRIANGULATION_PAIR_BY_INDEX);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());

		lastClickedRow = row;
	}
}

void TriangulationPairsGrid::redraw()
{
	auto rowCounter = 0;
	BeginBatch();
	DeleteRows(0, GetNumberRows());

	for (const auto& pair: *version->getTriangulationPairs())
	{
		auto bgColor = pair->getBaseRowColour();
		const auto& activeTriangulationPair = version->getActiveTriangulationPair();
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
	ForceRefresh();
}

void TriangulationPairsGrid::stageAddComment()
{
	auto* dialog = new TriangulationPairDialogComment(this, "Add Comment", lastClickedRow);
	dialog->ShowModal();
}

void TriangulationPairsGrid::rightUp(wxGridEvent& event)
{

	// Right up means deselect active link, which is serious stuff.
	// If our active link is dry, we're not deselecting it, we're deleting it.
	const wxCommandEvent* evt = new wxCommandEvent(wxEVT_DEACTIVATE_TRIANGULATION_PAIR);

	eventListener->QueueEvent(evt->Clone());
	event.Skip();
}

void TriangulationPairsGrid::activateLinkRowColor(int row)
{
	const auto& pair = version->getTriangulationPairs()->at(row);
	SetCellBackgroundColour(row, 0, pair->getActiveRowColour());
}

void TriangulationPairsGrid::restoreLinkRowColor(int row)
{
	const auto& pair = version->getTriangulationPairs()->at(row);
	SetCellBackgroundColour(row, 0, pair->getBaseRowColour());
}


void TriangulationPairsGrid::createTriangulationPair(int pairID)
{
	// Where is this new row?
	auto rowCounter = 0;
	for (const auto& pair: *version->getTriangulationPairs())
	{
		if (pair->getID() == pairID)
		{
			InsertRows(rowCounter, 1, false);
			SetCellValue(rowCounter, 0, pair->toRowString());

			activateLinkRowColor(rowCounter);
			// If we have an active link, restore its color.
			if (activeRow)
				restoreLinkRowColor(*activeRow + 1); // We have a link inserted, so we need to fix the following one.
			activeRow = rowCounter;
			lastClickedRow = rowCounter;
			// let's insert it.
			SetColMinimalWidth(0, 600);

			// If we only have a few rows, redraw to expand the grid height.
			if (GetNumberRows() < 6)
			{
				redraw();
				break;
			}

			ForceRefresh();
			break;
		}
		++rowCounter;
	}
}

void TriangulationPairsGrid::onUpdateComment(const wxCommandEvent& event)
{
	const auto comment = event.GetString().ToStdString();
	const auto index = event.GetInt();
	if (index < static_cast<int>(version->getTriangulationPairs()->size()))
	{
		const auto& pair = version->getTriangulationPairs()->at(index);
		pair->setComment(comment);
		// Also update screen.
		SetCellValue(index, 0, comment);
		ForceRefresh();
	}
}

void TriangulationPairsGrid::deactivateTriangulationPair()
{
	if (activeRow)
	{
		// Active pair may have been deleted by the LinkMapper. Check our records.
		if (static_cast<int>(version->getTriangulationPairs()->size()) == GetNumberRows())
		{
			// all is well, just deactivate.
			restoreLinkRowColor(*activeRow);
		}
		else
		{
			// We have a row too many. This is unacceptable.
			DeleteRows(*activeRow, 1, false);
			if (lastClickedRow > 0)
				--lastClickedRow;
		}
	}
	activeRow.reset();
	ForceRefresh();
}

void TriangulationPairsGrid::activatePairByID(const int ID)
{
	// We need to find not only which pair this is, but its row as well, so we can scroll the grid.
	// Thankfully, we're anal about their order.

	// If we're already active, restore color.
	if (activeRow)
		restoreLinkRowColor(*activeRow);

	auto rowCounter = 0;
	for (const auto& pair: *version->getTriangulationPairs())
	{
		if (pair->getID() == ID)
		{
			activeRow = rowCounter;
			activateLinkRowColor(rowCounter);
			if (!IsVisible(rowCounter, 0, false))
				focusOnActiveRow();
			lastClickedRow = rowCounter;
			break;
		}
		++rowCounter;
	}
}

void TriangulationPairsGrid::activatePairByIndex(const int index)
{
	// If we're already active, restore color.
	if (activeRow)
		restoreLinkRowColor(*activeRow);

	if (index >= static_cast<int>(version->getTriangulationPairs()->size()))
		return; // uh-huh

	activeRow = index;
	activateLinkRowColor(index);
	if (!IsVisible(index, 0, false))
		focusOnActiveRow();
	lastClickedRow = index;
}