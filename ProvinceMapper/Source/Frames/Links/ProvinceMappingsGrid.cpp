#include "ProvinceMappingsGrid.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);



ProvinceMappingsGrid::ProvinceMappingsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion) : wxGrid(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND), version(theVersion) {
	CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	EnableEditing(false);
	HideCellEditControl();
	HideRowLabels();
	HideColLabels();
	SetScrollRate(0, 10);
	SetColMinimalAcceptableWidth(600);
	GetGridWindow()->Bind(wxEVT_MOTION, &ProvinceMappingsGrid::onGridMotion, this);
	SetColMinimalWidth(0, 600);
}


void ProvinceMappingsGrid::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}


void ProvinceMappingsGrid::redraw()
{
	auto rowCounter = 0;
	BeginBatch();
	DeleteRows(0, GetNumberRows());

	for (const auto& link: *version->getLinks())
	{
		auto bgColor = wxColour(240, 240, 240);
		std::string name;
		std::string comma;
		if (link->getComment())
		{
			name = *link->getComment();
			bgColor = wxColour(150, 150, 150);
		}
		else
		{
			name = linkToString(link);
		}
		if (activeLink && *link == *activeLink)
		{
			if (link->getComment())
				bgColor = wxColour(50, 180, 50); // dark green for selected comments
			else
				bgColor = wxColour(150, 250, 150); // bright green for selected links.
			activeRow = rowCounter;
		}
		AppendRows(1, false);
		SetRowSize(rowCounter, 20);
		SetCellValue(rowCounter, 0, name);
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

std::string ProvinceMappingsGrid::linkToString(const std::shared_ptr<LinkMapping>& link)
{
	std::string name;
	std::string comma;
	for (const auto& source: link->getSources())
	{
		name += comma;
		if (source->locName)
			name += *source->locName;
		else if (!source->mapDataName.empty())
			name += source->mapDataName;
		else
			name += "(No Name)";
		comma = ", ";
	}
	name += " -> ";
	comma.clear();
	for (const auto& target: link->getTargets())
	{
		name += comma;
		if (target->locName)
			name += *target->locName;
		else if (!target->mapDataName.empty())
			name += target->mapDataName;
		else
			name += "(No Name)";
		comma = ", ";
	}
	return name;
}

void ProvinceMappingsGrid::leftUp(const wxGridEvent& event)
{
	// Left Up means:
	// 1. We want to mark a nonworking row as working row
	// 2. We are AGAIN clicking on a working row to center the map
	// 3. We're AGAIN clicking on a comment to change it.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(version->getLinks()->size()))
	{
		// Case 3: This is a comment.
		if (version->getLinks()->at(row)->getComment())
		{
			// and we're altering it.
			if (activeRow && *activeRow == row)
			{
				// spawn a dialog to change the name.
				auto* dialog = new DialogComment(this, "Edit Comment", *version->getLinks()->at(row)->getComment(), row);
				dialog->ShowModal();
				return;
			}
		}

		// Case 2: if we already clicked here, center the map.
		if (activeRow && *activeRow == row)
		{
			auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP);
			centerEvt->SetInt(activeLink->getID());
			eventListener->QueueEvent(centerEvt->Clone());
			return;
		}

		// Case 1: Selecting a new row.
		if (activeRow)
			restoreLinkRowColor(*activeRow);

		auto* evt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_INDEX);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());

		lastClickedRow = row;
	}
}



void ProvinceMappingsGrid::restoreLinkRowColor(int row) const
{
	const auto& link = version->getLinks()->at(row);
	if (link->getComment())
		theGrid->SetCellBackgroundColour(row, 0, wxColour(150, 150, 150)); // comment regular
	else
		theGrid->SetCellBackgroundColour(row, 0, wxColour(240, 240, 240)); // link regular
}

void ProvinceMappingsGrid::activateLinkRowColor(int row) const
{
	const auto& link = version->getLinks()->at(row);
	if (link->getComment())
		theGrid->SetCellBackgroundColour(row, 0, wxColour(50, 180, 50)); // comment highlight
	else
		theGrid->SetCellBackgroundColour(row, 0, wxColour(150, 250, 150)); // link highlight
}


void ProvinceMappingsGrid::deactivateLink()
{
	if (activeRow)
	{
		// Active link may have been deleted by linkmapper. Check our records.
		if (static_cast<int>(version->getLinks()->size()) == theGrid->GetNumberRows())
		{
			// all is well, just deactivate.
			restoreLinkRowColor(*activeRow);
		}
		else
		{
			// We have a row too many. This is unacceptable.
			theGrid->DeleteRows(*activeRow, 1, false);
			if (lastClickedRow > 0)
				--lastClickedRow;
		}
	}
	activeLink.reset();
	activeRow.reset();
	theGrid->ForceRefresh();
}

void ProvinceMappingsGrid::activateLinkByID(const int theID)
{
	// We need to find not only which link this is, but it's row as well so we can scroll the grid.
	// Thankfully, we're anal about their order.

	// If we're already active, restore color.
	if (activeRow)
		restoreLinkRowColor(*activeRow);

	auto rowCounter = 0;
	for (const auto& link: *version->getLinks())
	{
		if (link->getID() == theID)
		{
			activeRow = rowCounter;
			activeLink = link;
			activateLinkRowColor(rowCounter);
			if (!theGrid->IsVisible(rowCounter, 0, false))
				focusOnActiveRow();
			lastClickedRow = rowCounter;
			break;
		}
		++rowCounter;
	}
}

void ProvinceMappingsGrid::focusOnActiveRow()
{
	const auto cellCoords = theGrid->CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
	theGrid->Scroll(offset);														  // and shoo.
	theGrid->ForceRefresh();
}

void ProvinceMappingsGrid::refreshActiveLink()
{
	// this is called when we're toggling a province within the active link

	if (activeRow && activeLink)
	{
		const auto& name = linkToString(activeLink);
		theGrid->SetCellValue(*activeRow, 0, name);
	}
}

void ProvinceMappingsGrid::rightUp(wxGridEvent& event)
{
	const wxCommandEvent* evt;
	if (event.GetId() == theGrid->GetId())
	{
		// Right up means deselect active link, which is serious stuff.
		// If our active link is dry, we're not deselecting it, we're deleting it.
		evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
	}
	else
	{
		evt = new wxCommandEvent(wxEVT_DEACTIVATE_TRIANGULATION_PAIR); // TODO: move this to TriangulationPairsTab
	}
	eventListener->QueueEvent(evt->Clone());
	event.Skip();
}

void ProvinceMappingsGrid::createLink(const int linkID)
{
	// We could just redraw the entire grid but that flickers. This is more complicated but cleaner on the eyes.

	// Where is this new row?
	auto rowCounter = 0;
	for (const auto& link: *version->getLinks())
	{
		if (link->getID() == linkID)
		{
			theGrid->InsertRows(rowCounter, 1, false);
			if (link->getComment()) // this is a comment.
				theGrid->SetCellValue(rowCounter, 0, *link->getComment());
			else // new active link
				theGrid->SetCellValue(rowCounter, 0, linkToString(link));
			activateLinkRowColor(rowCounter);
			activeLink = link;
			// If we have an active link, restore its color.
			if (activeRow)
				restoreLinkRowColor(*activeRow + 1); // We have a link inserted so we need to fix the following one.
			activeRow = rowCounter;
			lastClickedRow = rowCounter;
			// let's insert it.
			theGrid->SetColMinimalWidth(0, 600);
			theGrid->ForceRefresh();
			break;
		}
		++rowCounter;
	}
}

void ProvinceMappingsGrid::stageAddComment()
{
	auto* dialog = new DialogComment(this, "Add Comment", lastClickedRow);
	dialog->ShowModal();
}



void ProvinceMappingsGrid::moveActiveLinkUp()
{
	if (activeLink && activeRow && *activeRow > 0)
	{
		const auto text = GetCellValue(*activeRow, 0);
		const auto color = GetCellBackgroundColour(*activeRow, 0);
		DeleteRows(*activeRow, 1, false);
		--*activeRow;
		InsertRows(*activeRow, 1, false);
		SetCellValue(*activeRow, 0, text);
		SetCellBackgroundColour(*activeRow, 0, color);
	}
}



void ProvinceMappingsGrid::moveActiveLinkDown()
{
	if (activeLink && activeRow && *activeRow < GetNumberRows() - 1)
	{
		const auto text = GetCellValue(*activeRow, 0);
		const auto color = GetCellBackgroundColour(*activeRow, 0);
		DeleteRows(*activeRow, 1, false);
		++*activeRow;
		InsertRows(*activeRow, 1, false);
		SetCellValue(*activeRow, 0, text);
		SetCellBackgroundColour(*activeRow, 0, color);
	}
}