#include "UnmappedTab.h"
#include "Frames/Images/ImageCanvas.h"
#include "Frames/Links/LinksTab.h"
#include "Frames/MainFrame.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_PROVINCE_CENTER_MAP, wxCommandEvent);

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
	theGrid->SetColMinimalAcceptableWidth(200);
	theGrid->GetGridWindow()->Bind(wxEVT_MOTION, &UnmappedTab::onGridMotion, this);
	theGrid->SetColMinimalWidth(0, 200);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	theGrid->ForceRefresh();
}

const std::vector<std::shared_ptr<Province>>& UnmappedTab::getRelevantProvinces() const
{
	if (selector == ImageTabSelector::SOURCE)
		return *version->getUnmappedSources();
	else
		return *version->getUnmappedTargets();
}

void UnmappedTab::redrawGrid() const
{
	auto rowCounter = 0;
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

	for (const auto& province: getRelevantProvinces())
	{
		auto bgColor = wxColour(240, 240, 240);
		std::string name = std::to_string(province->ID) + " - " + province->bespokeName();
		theGrid->AppendRows(1, false);
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
}

void UnmappedTab::leftUp(wxGridEvent& event)
{
	// Left Up means "center on this province". Not mark, not anything, just center.

	// We're selecting some cell. Let's translate that.
	const auto row = event.GetRow();
	if (row < static_cast<int>(getRelevantProvinces().size()))
	{
		auto centerEvt = wxCommandEvent(wxEVT_PROVINCE_CENTER_MAP);
		centerEvt.SetInt(getRelevantProvinces().at(row)->ID);
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
	auto* evt = new wxCommandEvent(wxMENU_ADD_COMMENT);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageDeleteLink() const
{
	// Do nothing unless working on active link. Don't want accidents here.
	auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveUp() const
{
	auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveDown() const
{
	auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageSave() const
{
	auto* evt = new wxCommandEvent(wxEVT_SAVE_LINKS);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageAddLink() const
{
	auto* evt = new wxCommandEvent(wxEVT_ADD_LINK);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveVersionLeft() const
{
	auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventListener->QueueEvent(evt->Clone());
}

void UnmappedTab::stageMoveVersionRight() const
{
	auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventListener->QueueEvent(evt->Clone());
}