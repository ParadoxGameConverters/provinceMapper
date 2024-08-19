#include "ProvinceMappingsGrid.h"
#include "DialogComment.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"



wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_REDRAW_LINKS_GRID, wxCommandEvent);



ProvinceMappingsGrid::ProvinceMappingsGrid(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion): GridBase(parent, std::move(theVersion))
{
	Bind(wxEVT_UPDATE_NAME, &ProvinceMappingsGrid::onUpdateComment, this);
}


void ProvinceMappingsGrid::redraw()
{
	auto rowCounter = 0;
	BeginBatch();
	DeleteRows(0, GetNumberRows());

	for (const auto& link: *version->getLinks())
	{
		auto bgColor = link->getBaseRowColour();
		std::string name = link->toRowString();
		const auto& activeLink = getActiveLink();
		if (activeLink && *link == *activeLink)
		{
			bgColor = link->getActiveRowColour();
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
			centerEvt->SetInt(getActiveLink()->getID());
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


void ProvinceMappingsGrid::activateLinkByIndex(const int index)
{
	// If we're already active, restore color.
	if (activeRow)
		restoreLinkRowColor(*activeRow);

	if (index >= static_cast<int>(version->getLinks()->size()))
		return; // uh-huh

	const auto& link = version->getLinks()->at(index);
	activeRow = index;
	activateLinkRowColor(index);
	if (!IsVisible(index, 0, false))
		focusOnActiveRow();
	lastClickedRow = index;
}


void ProvinceMappingsGrid::restoreLinkRowColor(int row)
{
	const auto& link = version->getLinks()->at(row);
	SetCellBackgroundColour(row, 0, link->getBaseRowColour());
}

void ProvinceMappingsGrid::activateLinkRowColor(int row)
{
	const auto& link = version->getLinks()->at(row);
	SetCellBackgroundColour(row, 0, link->getActiveRowColour());
}


void ProvinceMappingsGrid::deactivateLink()
{
	if (activeRow)
	{
		// Active link may have been deleted by linkmapper. Check our records.
		if (static_cast<int>(version->getLinks()->size()) == GetNumberRows())
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
			activateLinkRowColor(rowCounter);
			if (!IsVisible(rowCounter, 0, false))
				focusOnActiveRow();
			lastClickedRow = rowCounter;
			break;
		}
		++rowCounter;
	}
}

void ProvinceMappingsGrid::rightUp(wxGridEvent& event)
{
	// Right up means deselect active link, which is serious stuff.
	// If our active link is dry, we're not deselecting it, we're deleting it.
	const wxCommandEvent* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);

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
			InsertRows(rowCounter, 1, false);
			SetCellValue(rowCounter, 0, link->toRowString());
			activateLinkRowColor(rowCounter);
			// If we have an active link, restore its color.
			if (activeRow)
				restoreLinkRowColor(*activeRow + 1); // We have a link inserted so we need to fix the following one.
			activeRow = rowCounter;
			lastClickedRow = rowCounter;
			// let's insert it.
			SetColMinimalWidth(0, 600);
			ForceRefresh();
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

void ProvinceMappingsGrid::onUpdateComment(const wxCommandEvent& event)
{
	const auto comment = event.GetString().ToStdString();
	const auto index = event.GetInt();
	if (index < static_cast<int>(version->getLinks()->size()))
	{
		const auto& link = version->getLinks()->at(index);
		link->setComment(comment);
		// Also update screen.
		SetCellValue(index, 0, comment);
		ForceRefresh();
	}
}