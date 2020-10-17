#include "LinksTab.h"
#include "DialogComment.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);

LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), ID(theID), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &LinksTab::leftUp, this);
	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &LinksTab::rightUp, this);
	Bind(wxEVT_UPDATE_COMMENT, &LinksTab::onUpdateComment, this);

	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->SetColLabelValue(0, version->getName());
	theGrid->SetColLabelAlignment(wxLEFT, wxCENTER);
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColLabelSize(20);
	theGrid->SetMinSize(wxSize(600, 900));
	theGrid->GetGridWindow()->Bind(wxEVT_MOTION, &LinksTab::onGridMotion, this);
	GetParent()->Layout();

	auto* logBox = new wxBoxSizer(wxVERTICAL);
	logBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(logBox);
	logBox->Fit(this);
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
			if (activeLink && *link == *activeLink)
			{
				bgColor = wxColour(150, 250, 150);
				activeRow = rowCounter;
			}
			name = linkToString(link);
		}
		theGrid->AppendRows(1, false);
		theGrid->SetRowSize(rowCounter, 20);
		theGrid->SetCellValue(rowCounter, 0, name);
		theGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		theGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		theGrid->SetReadOnly(rowCounter, 0);
		rowCounter++;
	}
	theGrid->EndBatch();
	theGrid->AutoSize();
	if (activeRow)
		focusOnActiveRow();
	GetParent()->Layout();
}

std::string LinksTab::linkToString(const std::shared_ptr<LinkMapping>& link)
{
	std::string name;
	std::string comma;
	for (const auto& source: link->getSources())
	{
		name += comma;
		if (!source->locName.empty())
			name += source->locName;
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
		if (!target->locName.empty())
			name += target->locName;
		if (!target->mapDataName.empty())
			name += target->mapDataName;
		else
			name += "(No Name)";
		comma = ", ";
	}
	return name;
}

void LinksTab::leftUp(wxGridEvent& event)
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
			// deactivate any potential working link.
			auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
			eventListener->QueueEvent(evt->Clone());
			if (row == lastClickedRow)
			{
				// spawn a dialog to change the name.
				auto* dialog = new DialogComment(this, "Edit Comment", *version->getLinks()->at(row)->getComment(), row);
				dialog->ShowModal();
			}
			lastClickedRow = row;
			return;
		}

		// Case 1: Selecting a new row.
		// Deselect existing working link
		if (activeRow)
			theGrid->SetCellBackgroundColour(*activeRow, 0, wxColour(240, 240, 240));

		// And mark the new working row.
		activeLink = version->getLinks()->at(row);
		activeRow = row;
		theGrid->SetCellBackgroundColour(*activeRow, 0, wxColour(150, 250, 150));
		Refresh();

		auto* evt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_INDEX);
		evt->SetInt(row);
		eventListener->QueueEvent(evt->Clone());

		// Case 2: if we already clicked here, center the map.
		if (row == lastClickedRow)
		{
			auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP);
			centerEvt->SetInt(row);
			eventListener->QueueEvent(centerEvt->Clone());
		}

		lastClickedRow = row;
	}
}

void LinksTab::deactivateLink()
{
	if (activeRow)
		theGrid->SetCellBackgroundColour(*activeRow, 0, wxColour(240, 240, 240));
	activeLink.reset();
	activeRow.reset();
	Refresh();
}

void LinksTab::activateLinkByID(const int theID)
{
	// We need to find not only which link this is, but it's row as well so we can scroll the grid.
	// Thankfully, we're anal about their order.

	auto rowCounter = 0;
	for (const auto& link: *version->getLinks())
	{
		if (link->getID() == theID)
		{
			activeRow = rowCounter;
			activeLink = link;
			focusOnActiveRow();
			break;
		}
		++rowCounter;
	}
	Refresh();
}

void LinksTab::focusOnActiveRow()
{
	theGrid->SetCellBackgroundColour(*activeRow, 0, wxColour(150, 250, 150));
	const auto cellCoords = theGrid->CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
	theGrid->Scroll(offset);														  // and shoo.
}

void LinksTab::refreshActiveLink()
{
	if (activeRow && activeLink)
	{
		const auto& name = linkToString(activeLink);
		theGrid->SetCellValue(*activeRow, 0, name);
		Refresh();
	}
}

void LinksTab::rightUp(wxGridEvent& event)
{
	// Right up means deselect active link, nothing else.
	auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
	eventListener->QueueEvent(evt->Clone());
	event.Skip();
}

void LinksTab::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}

void LinksTab::onUpdateComment(wxCommandEvent& event)
{
	const auto comment = event.GetString().ToStdString();
	const auto index = event.GetInt();
	if (index < static_cast<int>(version->getLinks()->size()))
	{
		const auto& link = version->getLinks()->at(index);
		link->setComment(comment);
		// also update screen.
		theGrid->SetCellValue(index, 0, comment);
		Refresh();
	}
}
