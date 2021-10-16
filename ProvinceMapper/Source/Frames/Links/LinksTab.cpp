#include "LinksTab.h"
#include "DialogComment.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_HIGHLIGHT_REGION, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DISABLE_REGION_HIGHLIGHT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);

LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &LinksTab::leftUp, this);
	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &LinksTab::rightUp, this);
	Bind(wxEVT_UPDATE_NAME, &LinksTab::onUpdateComment, this);
	Bind(wxEVT_KEY_DOWN, &LinksTab::onKeyDown, this);

	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->EnableEditing(false);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->HideColLabels();
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColMinimalAcceptableWidth(600);
	theGrid->GetGridWindow()->Bind(wxEVT_MOTION, &LinksTab::onGridMotion, this);
	theGrid->SetColMinimalWidth(0, 600);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	theGrid->ForceRefresh();
}

void LinksTab::redrawGrid()
{
	auto rowCounter = 0;
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

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
		theGrid->AppendRows(1, false);
		theGrid->SetRowSize(rowCounter, 20);
		theGrid->SetCellValue(rowCounter, 0, name);
		theGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		theGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		rowCounter++;
	}
	theGrid->AutoSizeColumn(0, false);
	theGrid->EndBatch();
	if (activeRow)
		focusOnActiveRow();
	GetParent()->Layout();
	theGrid->ForceRefresh();
}

std::string LinksTab::linkToString(const std::shared_ptr<LinkMapping>& link)
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

void LinksTab::leftUp(const wxGridEvent& event)
{
	// Left Up means:
	// 1. We want to mark a nonworking row as working row
	// 2. We are AGAIN clicking on a working row to center the map
	// 3. We're AGAIN clicking on a comment to change it.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	const auto& links = version->getLinks();
	const auto& clickedLink = links->at(row);
	if (row >= static_cast<int>(links->size()))
		return;
	// Case 3: This is a comment.
	if (clickedLink->getComment())
	{
		// and we're altering it.
		if (activeRow && *activeRow == row)
		{
			// spawn a dialog to change the name.
			auto* dialog = new DialogComment(this, "Edit Comment", *clickedLink->getComment(), row);
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
		restoreRowColor(*activeRow);

	auto* evt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_INDEX);
	evt->SetInt(row);
	eventListener->QueueEvent(evt->Clone());

	lastClickedRow = row;

	// if the selected row's link is a comment
	// highlight provinces from all links between the selected comment and the first comment below it
	if (clickedLink->getComment())
	{
		auto* evt = new wxCommandEvent(wxEVT_HIGHLIGHT_REGION);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::restoreRowColor(const int row) const
{
	const auto& link = version->getLinks()->at(row);
	if (link->getComment())
		theGrid->SetCellBackgroundColour(row, 0, wxColour(150, 150, 150)); // comment regular
	else
		theGrid->SetCellBackgroundColour(row, 0, wxColour(240, 240, 240)); // link regular
}

void LinksTab::activateRowColor(const int row) const
{
	const auto& link = version->getLinks()->at(row);
	if (link->getComment())
		theGrid->SetCellBackgroundColour(row, 0, wxColour(50, 180, 50)); // comment highlight
	else
		theGrid->SetCellBackgroundColour(row, 0, wxColour(150, 250, 150)); // link highlight
}


void LinksTab::deactivateLink()
{
	if (activeRow)
	{
		// Active link may have been deleted by linkmapper. Check our records.
		if (static_cast<int>(version->getLinks()->size()) == theGrid->GetNumberRows())
		{
			// all is well, just deactivate.
			restoreRowColor(*activeRow);
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

void LinksTab::activateLinkByID(const int theID)
{
	// We need to find not only which link this is, but it's row as well so we can scroll the grid.
	// Thankfully, we're anal about their order.

	// If we're already active, restore color.
	if (activeRow)
		restoreRowColor(*activeRow);

	auto rowCounter = 0;
	for (const auto& link: *version->getLinks())
	{
		if (link->getID() == theID)
		{
			activeRow = rowCounter;
			activeLink = link;
			activateRowColor(rowCounter);
			if (!theGrid->IsVisible(rowCounter, 0, false))
				focusOnActiveRow();
			lastClickedRow = rowCounter;
			break;
		}
		++rowCounter;
	}
}

void LinksTab::activateLinkByIndex(const int index)
{
	// If we're already active, restore color.
	if (activeRow)
		restoreRowColor(*activeRow);

	if (index >= static_cast<int>(version->getLinks()->size()))
		return; // uh-huh

	const auto& link = version->getLinks()->at(index);
	activeRow = index;
	activeLink = link;
	activateRowColor(index);
	if (!theGrid->IsVisible(index, 0, false))
		focusOnActiveRow();
	lastClickedRow = index;
}

void LinksTab::focusOnActiveRow()
{
	const auto cellCoords = theGrid->CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
	theGrid->Scroll(offset);														  // and shoo.
	theGrid->ForceRefresh();
}

void LinksTab::refreshActiveLink()
{
	// this is called when we're toggling a province within the active link

	if (activeRow && activeLink)
	{
		const auto& name = linkToString(activeLink);
		theGrid->SetCellValue(*activeRow, 0, name);
	}
}

void LinksTab::rightUp(wxGridEvent& event)
{
	// Right up means deselect active link, which is serious stuff.
	// If our active link is dry, we're not deselecting it, we're deleting it.
	const auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
	eventListener->QueueEvent(evt->Clone());

	// If the link is a comment, disable the region highlight
	evt = new wxCommandEvent(wxEVT_DISABLE_REGION_HIGHLIGHT);
	eventListener->QueueEvent(evt->Clone());

	event.Skip();
}

void LinksTab::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}

void LinksTab::onUpdateComment(const wxCommandEvent& event)
{
	const auto comment = event.GetString().ToStdString();
	const auto index = event.GetInt();
	if (index < static_cast<int>(version->getLinks()->size()))
	{
		const auto& link = version->getLinks()->at(index);
		link->setComment(comment);
		// also update screen.
		theGrid->SetCellValue(index, 0, comment);
		theGrid->ForceRefresh();
	}
}

void LinksTab::createLink(const int linkID)
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
			activateRowColor(rowCounter);
			activeLink = link;
			// If we have an active link, restore its color.
			if (activeRow)
				restoreRowColor(*activeRow + 1); // We have a link inserted so we need to fix the following one.
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

void LinksTab::onKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_F3:
			stageAddLink();
			break;
		case WXK_F4:
			// spawn a dialog to name the comment.
			stageAddComment();
			break;
		case WXK_F5:
			stageSave();
			break;
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
			stageDeleteLink();
			break;
		case WXK_NUMPAD_SUBTRACT:
			stageMoveUp();
			break;
		case WXK_NUMPAD_ADD:
			stageMoveDown();
			break;
		case WXK_NUMPAD_MULTIPLY:
			stageMoveVersionRight();
			break;
		case WXK_NUMPAD_DIVIDE:
			stageMoveVersionLeft();
			break;
		default:
			event.Skip();
	}
}

void LinksTab::stageAddComment()
{
	auto* dialog = new DialogComment(this, "Add Comment", lastClickedRow);
	dialog->ShowModal();
}

void LinksTab::stageDeleteLink() const
{
	// Do nothing unless working on active link. Don't want accidents here.
	if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveUp() const
{
	if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveDown() const
{
	if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::moveActiveLinkUp()
{
	if (activeLink && activeRow && *activeRow > 0)
	{
		const auto text = theGrid->GetCellValue(*activeRow, 0);
		const auto color = theGrid->GetCellBackgroundColour(*activeRow, 0);
		theGrid->DeleteRows(*activeRow, 1, false);
		--*activeRow;
		theGrid->InsertRows(*activeRow, 1, false);
		theGrid->SetCellValue(*activeRow, 0, text);
		theGrid->SetCellBackgroundColour(*activeRow, 0, color);
	}
}

void LinksTab::moveActiveLinkDown()
{
	if (activeLink && activeRow && *activeRow < theGrid->GetNumberRows() - 1)
	{
		const auto text = theGrid->GetCellValue(*activeRow, 0);
		const auto color = theGrid->GetCellBackgroundColour(*activeRow, 0);
		theGrid->DeleteRows(*activeRow, 1, false);
		++*activeRow;
		theGrid->InsertRows(*activeRow, 1, false);
		theGrid->SetCellValue(*activeRow, 0, text);
		theGrid->SetCellBackgroundColour(*activeRow, 0, color);
	}
}

void LinksTab::stageSave() const
{
	const auto* evt = new wxCommandEvent(wxEVT_SAVE_LINKS);
	eventListener->QueueEvent(evt->Clone());
}

void LinksTab::stageAddLink() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void LinksTab::stageMoveVersionLeft() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventListener->QueueEvent(evt->Clone());
}

void LinksTab::stageMoveVersionRight() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventListener->QueueEvent(evt->Clone());
}
