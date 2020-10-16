#include "LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);

LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), ID(theID), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &LinksTab::onCellSelect, this);

	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->SetColLabelValue(0, version->getName());
	theGrid->SetColLabelAlignment(wxLEFT, wxCENTER);
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColLabelSize(20);

	theGrid->SetMinSize(wxSize(600, 900));
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
		if (!link->getComment().empty())
		{
			name = link->getComment();
			bgColor = wxColour(150, 150, 150);
		}
		else
		{
			if (activeLink && *link == *activeLink)
			{
				bgColor = wxColour(150, 250, 150);
				activeRow = rowCounter;
			}
			for (const auto& source: link->getSources())
			{
				name += comma;
				if (source->mapDataName.empty())
					name += "(No Name)";
				else
					name += source->mapDataName;
				comma = ", ";
			}
			name += " -> ";
			comma.clear();
			for (const auto& target: link->getTargets())
			{
				name += comma;
				if (target->mapDataName.empty())
					name += "(No Name)";
				else
					name += target->mapDataName;
				comma = ", ";
			}
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
		theGrid->MakeCellVisible(*activeRow, 0);
	GetParent()->Layout();
}

void LinksTab::onCellSelect(wxGridEvent& event)
{
	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(version->getLinks()->size()))
	{
		// Don't workify links that aren't links.
		if (!version->getLinks()->at(row)->getComment().empty())
		{
			activeLink.reset();
			activeRow.reset();
			auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
			eventListener->QueueEvent(evt->Clone());
			return;
		}

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
			theGrid->SetCellBackgroundColour(*activeRow, 0, wxColour(150, 250, 150));
			const auto cellCoords = theGrid->CellToRect(*activeRow, 0);			  // these would be virtual coords, not logical ones.
			const auto units = cellCoords.y / 20;										  // pixels into scroll units, 20 is our scroll rate defined in constructor.
			const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL); // this is how much "scrolls" a pageful of cells scrolls.
			const auto offset = wxPoint(0, units - scrollPageSize / 2);			  // position ourselves at our cell, minus half a screen of scrolls.
			theGrid->Scroll(offset);														  // and shoo.
			break;
		}
		++rowCounter;
	}
	Refresh();
}
