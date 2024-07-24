#include "LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"
#include "TriangulationPairsGrid.h"
#include "ProvinceMappingsGrid.h"


wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);


LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &LinksTab::leftUp, this); // TODO: move this to GridBase
	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &LinksTab::rightUp, this); // TODO: move this to GridBase
	Bind(wxEVT_UPDATE_NAME, &LinksTab::onUpdateComment, this);
	Bind(wxEVT_KEY_DOWN, &LinksTab::onKeyDown, this);

	wxStaticText* pairsTitle = new wxStaticText(this, wxID_ANY, "Triangulation Pairs", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	pairsTitle->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	triangulationPointGrid = new TriangulationPairsGrid(this, version);
	GetParent()->Layout();

	wxStaticText* linksTitle = new wxStaticText(this, wxID_ANY, "Province Links", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	linksTitle->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	provinceMappingsGrid = new ProvinceMappingsGrid(this, version);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(pairsTitle, 0, wxALIGN_CENTER | wxALL, 10);
	gridBox->Add(triangulationPointGrid, 1, wxEXPAND | wxALL);

	gridBox->AddSpacer(20); // Visually separate the triangulation pairs grid from the province links grid.
	gridBox->Add(pairsTitle, 0, wxALIGN_CENTER | wxALL, 10);
	gridBox->Add(provinceMappingsGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	triangulationPointGrid->ForceRefresh();
	provinceMappingsGrid->ForceRefresh();
}


void LinksTab::redraw()
{
	triangulationPointGrid->redraw();
	provinceMappingsGrid->redraw();
}

void LinksTab::leftUp(const wxGridEvent& event)
{
	if (event.GetId() == provinceMappingsGrid->GetId()) // TODO: REMOVE THIS IF BECAUSE THE GRID ARE BEING EXTRACTED TO SEPARATE FILES
	{
		provinceMappingsGrid->leftUp(event);
	}
	else
	{
		triangulationPairsGridLeftUp(event);
	}
}

void LinksTab::restoreTriangulationPairRowColor(int pairRow) const
{
	const auto& pair = version->getTriangulationPointPairs()->at(pairRow);
	triangulationPointGrid->SetCellBackgroundColour(pairRow, 0, pair->getBaseRowColour()); // link regular
}

void LinksTab::activateTriangulationPairRowColor(int pairRow) const
{
	const auto& pair = version->getTriangulationPointPairs()->at(pairRow);
	triangulationPointGrid->SetCellBackgroundColour(pairRow, 0, pair->getActiveRowColour()); // link highlight
}

void LinksTab::activateLinkByIndex(const int index)
{
	provinceMappingsGrid->activateLinkByIndex(index);
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
		provinceMappingsGrid->SetCellValue(index, 0, comment);
		provinceMappingsGrid->ForceRefresh();
	}
}

void LinksTab::createLink(int linkID)
{
	provinceMappingsGrid->createLink(linkID);
}

void LinksTab::createTriangulationPair(int pairID)
{
	triangulationPointGrid->createTriangulationPair(pairID);
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
		case WXK_F6:
			stageAddTriangulationPair();
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
	if (triangulationPointGrid->activeLink)
	{
		triangulationPointGrid->stageAddComment();
	}
	else
	{
		provinceMappingsGrid->stageAddComment();
	}
}

void LinksTab::stageDeleteLink() const
{
	if (triangulationPointGrid->activeLink) {
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR);
		eventListener->QueueEvent(evt->Clone());
	}
	// Do nothing unless working on active link. Don't want accidents here.
	else if (provinceMappingsGrid->activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveUp() const
{	
	if (triangulationPointGrid->activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_UP);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (provinceMappingsGrid->activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveDown() const
{
	if (triangulationPointGrid->activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (provinceMappingsGrid->activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::moveActiveLinkUp()
{
	// TODO: handle triangulation pairs 
	provinceMappingsGrid->moveActiveLinkUp();
}

void LinksTab::moveActiveLinkDown()
{
	// TODO: handle triangulation pairs 
	provinceMappingsGrid->moveActiveLinkDown();
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

void LinksTab::stageAddTriangulationPair() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_TRIANGULATION_PAIR);
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
