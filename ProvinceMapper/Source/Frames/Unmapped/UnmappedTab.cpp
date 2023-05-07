#include "UnmappedTab.h"
#include "Frames/Images/ImageCanvas.h"
#include "Frames/Links/LinksTab.h"
#include "Frames/MainFrame.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"
#include <ranges>

wxDEFINE_EVENT(wxEVT_PROVINCE_CENTER_MAP, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_UPDATE_PROVINCE_COUNT, wxCommandEvent);

UnmappedTab::UnmappedTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, ImageTabSelector theSelector):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), selector(theSelector), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &UnmappedTab::leftUp, this);
	Bind(wxEVT_KEY_DOWN, &UnmappedTab::onKeyDown, this);

	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->EnableEditing(false);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->HideColLabels();
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColMinimalAcceptableWidth(600);
	theGrid->GetGridWindow()->Bind(wxEVT_MOTION, &UnmappedTab::onGridMotion, this);
	theGrid->SetColMinimalWidth(0, 600);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	theGrid->ForceRefresh();
}

const std::vector<std::shared_ptr<Province>> UnmappedTab::getRelevantProvinces() const
{
	std::vector<std::shared_ptr<Province>> relevantProvinces;
	if (selector == ImageTabSelector::SOURCE)
		relevantProvinces = *version->getUnmappedSources();
	else
		relevantProvinces = *version->getUnmappedTargets();

	if (excludeWaterProvinces)
	{
		std::vector<std::shared_ptr<Province>> filteredProvinces;
		for (auto &p : relevantProvinces | std::views::filter([](std::shared_ptr<Province> p){ return !p->isWater(); })) {
			filteredProvinces.push_back(p);
		}
		relevantProvinces = filteredProvinces;
	}
	if (excludeImpassables)
	{
		std::vector<std::shared_ptr<Province>> filteredProvinces;
		for (auto &p : relevantProvinces | std::views::filter([](std::shared_ptr<Province> p){ return !p->isImpassable(); })) {
			filteredProvinces.push_back(p);
		}
		relevantProvinces = filteredProvinces;
	}

	return relevantProvinces;

	
}

void UnmappedTab::redrawGrid()
{
	auto rowCounter = 0;
	provinceRows.clear();
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

	for (const auto& province: getRelevantProvinces())
	{
		const auto bgColor = wxColour(240, 240, 240);
		const auto name = province->ID + " - " + province->bespokeName();
		theGrid->AppendRows(1, false);
		provinceRows.insert(std::pair(province->ID, rowCounter));
		theGrid->SetRowSize(rowCounter, 20);
		theGrid->SetCellValue(rowCounter, 0, name);
		theGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		theGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		rowCounter++;
	}
	theGrid->AutoSizeColumn(0, false);
	theGrid->EndBatch();
	GetParent()->Layout();
	theGrid->ForceRefresh();
	stageUpdateProvinceCount();
}

void UnmappedTab::stageUpdateProvinceCount() const
{
	auto evt = wxCommandEvent(wxEVT_UPDATE_PROVINCE_COUNT);
	evt.SetInt(theGrid->GetNumberRows());
	if (selector == ImageTabSelector::SOURCE)
		evt.SetId(0);
	else
		evt.SetId(1);
	eventListener->AddPendingEvent(evt);
}

void UnmappedTab::leftUp(const wxGridEvent& event)
{
	// Left Up means "center on this province". Not mark, not anything, just center.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(getRelevantProvinces().size()))
	{
		auto centerEvt = wxCommandEvent(wxEVT_PROVINCE_CENTER_MAP);
		centerEvt.SetString(getRelevantProvinces().at(row)->ID);
		if (selector == ImageTabSelector::SOURCE)
			centerEvt.SetId(0);
		else
			centerEvt.SetId(1);
		eventListener->AddPendingEvent(centerEvt);
	}
}

void UnmappedTab::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}

void UnmappedTab::onKeyDown(wxKeyEvent& event)
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

void UnmappedTab::stageAddComment() const
{
	const auto* evt = new wxCommandEvent(wxMENU_ADD_COMMENT);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageDeleteLink() const
{
	// Do nothing unless working on active link. Don't want accidents here.
	const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveUp() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveDown() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageSave() const
{
	const auto* evt = new wxCommandEvent(wxEVT_SAVE_LINKS);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageAddLink() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveVersionLeft() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveVersionRight() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::removeProvince(const std::string& ID)
{
	if (const auto& provinceRowItr = provinceRows.find(ID); provinceRowItr != provinceRows.end())
	{
		const auto row = provinceRowItr->second;
		theGrid->DeleteRows(provinceRowItr->second, 1);
		provinceRows.erase(ID);
		for (auto& provinceRow: provinceRows) // lower the row position for ones behind.
			if (provinceRow.second > row)
				--provinceRow.second;
		theGrid->ForceRefresh();
		stageUpdateProvinceCount();
	}
}

void UnmappedTab::addProvince(const std::string& ID)
{
	for (const auto& province: getRelevantProvinces())
	{
		if (province->ID == ID)
		{
			const auto bgColor = wxColour(240, 240, 240);
			const auto name = province->ID + " - " + province->bespokeName();
			auto currentRow = theGrid->GetNumberRows();
			theGrid->AppendRows(1);
			provinceRows.insert(std::pair(province->ID, currentRow));
			theGrid->SetCellValue(currentRow, 0, name);
			theGrid->SetCellBackgroundColour(currentRow, 0, bgColor);
			theGrid->SetCellAlignment(currentRow, 0, wxCENTER, wxCENTER);
			theGrid->SetColMinimalWidth(0, 600);
			theGrid->ForceRefresh();
			focusOnRow(currentRow);
			stageUpdateProvinceCount();
			break;
		}
	}
}

void UnmappedTab::setExcludeWaterProvinces(bool excludeWaterProvinces)
{
	this->excludeWaterProvinces = excludeWaterProvinces;
	redrawGrid();
}

void UnmappedTab::setExcludeImpassables(bool excludeImpassables)
{
	this->excludeImpassables = excludeImpassables;
	redrawGrid();
}


void UnmappedTab::focusOnRow(const int row)
{
	const auto cellCoords = theGrid->CellToRect(row, 0);
	const auto units = cellCoords.y / 20;
	const auto scrollPageSize = theGrid->GetScrollPageSize(wxVERTICAL);
	const auto offset = wxPoint(0, units - scrollPageSize / 2);
	theGrid->Scroll(offset);
	theGrid->ForceRefresh();
}
