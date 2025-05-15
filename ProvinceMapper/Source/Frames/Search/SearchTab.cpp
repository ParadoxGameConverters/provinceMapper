#include "SearchTab.h"
#include "Frames/Images/ImageCanvas.h"
#include "Frames/Links/LinksTab.h"
#include "Frames/Links/ProvinceMappingsGrid.h"
#include "Frames/Links/TriangulationPairsGrid.h"
#include "Frames/MainFrame.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Provinces/Province.h"
#include <ranges>
#include <algorithm>
#include <string>


SearchTab::SearchTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &SearchTab::leftUp, this);
	Bind(wxEVT_KEY_DOWN, &SearchTab::onKeyDown, this);

	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->EnableEditing(false);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->HideColLabels();
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColMinimalAcceptableWidth(600);
	theGrid->GetGridWindow()->Bind(wxEVT_MOTION, &SearchTab::onGridMotion, this);
	theGrid->SetColMinimalWidth(0, 600);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	theGrid->ForceRefresh();
}

/// Try to find in the Haystack the Needle - ignore case
static bool findStringIC(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(strHaystack.begin(), strHaystack.end(), strNeedle.begin(), strNeedle.end(), [](unsigned char ch1, unsigned char ch2) {
		return std::toupper(ch1) == std::toupper(ch2);
	});
	return (it != strHaystack.end());
}

const std::vector<std::shared_ptr<LinkMapping>> SearchTab::getRelevantLinks() const // TODO: CHECK IF THIS RETURNS GOOD RESULTS
{
	// If the search string is empty, return an empty vector.
	if (searchString.empty())
	{
		return {};
	}

	std::vector<std::shared_ptr<LinkMapping>> matchingLinks;

   for (auto& link: *version->getLinks() | std::views::filter([this](const std::shared_ptr<LinkMapping>& l) {
			  return findStringIC(l->toRowStringForSearchFrame(), searchString);
	}))
	{
		matchingLinks.push_back(link);
	}

	return matchingLinks;
}

void SearchTab::redrawGrid()
{
	auto rowCounter = 0;
	linkRows.clear();
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

	for (const auto& link: getRelevantLinks())
	{
		const auto bgColor = wxColour(240, 240, 240);
		theGrid->AppendRows(1, false);
		linkRows.insert(std::pair(link->getID(), rowCounter));
		theGrid->SetRowSize(rowCounter, 20);
		theGrid->SetCellValue(rowCounter, 0, link->toRowStringForSearchFrame());
		theGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		theGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		rowCounter++;
	}
	theGrid->AutoSizeColumn(0, false);
	theGrid->EndBatch();
	GetParent()->Layout();
	theGrid->ForceRefresh();
}

void SearchTab::leftUp(const wxGridEvent& event) // TODO: check if this correctly focuses the map on the clicked link
{
	// Left Up means "center on this link". Not mark, not anything, just center.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(getRelevantLinks().size()))
	{
		const int linkID = getRelevantLinks().at(row)->getID();

		auto* centerEvt = new wxCommandEvent(wxEVT_CENTER_MAP);
		centerEvt->SetInt(linkID);
		eventListener->QueueEvent(centerEvt->Clone());

		auto* selectLinkEvt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_ID);
		selectLinkEvt->SetInt(linkID);
		eventListener->QueueEvent(selectLinkEvt->Clone());
	}
}

void SearchTab::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}

void SearchTab::onKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_F3:
			stageAddLink();
			break;
		case WXK_F4:
			stageAddComment();
			break;
		case WXK_F5:
			stageSave();
			break;
		case WXK_F6:
			stageAddTriangulationPair();
			break;
		case WXK_F7:
			stageAutogenerateMappings();
			break;
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
			stageDeleteLinkOrTriangulationPair();
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

void SearchTab::stageAddComment() const
{
	const auto* evt = new wxCommandEvent(wxMENU_ADD_COMMENT);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageDeleteLinkOrTriangulationPair() const
{
	// Do nothing unless working on active link. Don't want accidents here.
	const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageMoveUp() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageMoveDown() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageSave() const
{
	const auto* evt = new wxCommandEvent(wxEVT_SAVE_LINKS);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageAddLink() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageAddTriangulationPair() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_TRIANGULATION_PAIR);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageAutogenerateMappings() const
{
	const auto* evt = new wxCommandEvent(wxEVT_AUTOGENERATE_MAPPINGS);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageMoveVersionLeft() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::stageMoveVersionRight() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventListener->QueueEvent(evt->Clone());
}

void SearchTab::setSearchString(const std::string& searchStr)
{
	this->searchString = searchStr;
	redrawGrid();
}

void SearchTab::focusOnRow(const int row)
{
	const auto cellCoords = theGrid->CellToRect(row, 0);
	const auto units = cellCoords.y / 20;
	const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL);
	const auto offset = wxPoint(0, units - scrollPageSize / 2);
	theGrid->Scroll(offset);
	theGrid->ForceRefresh();
}
