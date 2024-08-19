#include "LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"
#include "TriangulationPairsGrid.h"
#include "ProvinceMappingsGrid.h"

#include <wx/splitter.h>
#include <wx/generic/splitter.h>


wxDEFINE_EVENT(wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);


LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), version(std::move(theVersion)), eventListener(parent)
{
	Bind(wxEVT_KEY_DOWN, &LinksTab::onKeyDown, this);

   auto* sizer = new wxBoxSizer(wxVERTICAL);

	auto* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxEXPAND);

	auto* triangulationPairsPanel = new wxPanel(splitter, wxID_ANY);
	auto* triangulationPairsPanelSizer = new wxBoxSizer(wxVERTICAL);
	auto* pairsTitle = new wxStaticText(triangulationPairsPanel, wxID_ANY, "Triangulation Pairs", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	pairsTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	triangulationPointGrid = new TriangulationPairsGrid(triangulationPairsPanel, version);
	triangulationPairsPanelSizer->Add(pairsTitle, 0, wxEXPAND);
	triangulationPairsPanelSizer->Add(triangulationPointGrid, 1, wxEXPAND);
	triangulationPairsPanel->SetSizer(triangulationPairsPanelSizer);

   auto* linksPanel = new wxPanel(splitter, wxID_ANY);
	auto* linksPanelSizer = new wxBoxSizer(wxVERTICAL);
	auto* linksTitle = new wxStaticText(linksPanel, wxID_ANY, "Province Links", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	linksTitle->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	provinceMappingsGrid = new ProvinceMappingsGrid(linksPanel, version);
	linksPanelSizer->Add(linksTitle, 0, wxEXPAND);
	linksPanelSizer->Add(provinceMappingsGrid, 1, wxEXPAND);
	linksPanel->SetSizer(linksPanelSizer);

	splitter->SplitHorizontally(triangulationPairsPanel, linksPanel);
	splitter->SetSashInvisible(false);
	splitter->SetSashGravity(0.5);
	splitter->SetSashPosition(200);
	sizer->Add(splitter, 1, wxEXPAND);

   this->SetSizer(sizer);

	triangulationPointGrid->ForceRefresh();
	provinceMappingsGrid->ForceRefresh();
}


void LinksTab::redraw() const
{
	triangulationPointGrid->redraw();
	redrawProvinceLinksGrid();
}

void LinksTab::redrawProvinceLinksGrid() const
{
	provinceMappingsGrid->redraw();
}

void LinksTab::restoreTriangulationPairRowColor(int pairRow) const
{
	const auto& pair = version->getTriangulationPairs()->at(pairRow);
	triangulationPointGrid->SetCellBackgroundColour(pairRow, 0, pair->getBaseRowColour()); // link regular
}

void LinksTab::activateTriangulationPairRowColor(int pairRow) const
{
	const auto& pair = version->getTriangulationPairs()->at(pairRow);
	triangulationPointGrid->SetCellBackgroundColour(pairRow, 0, pair->getActiveRowColour()); // link highlight
}

void LinksTab::deactivateLink()
{
	provinceMappingsGrid->deactivateLink();
}

void LinksTab::deactivateTriangulationPair()
{
	triangulationPointGrid->deactivateTriangulationPair();
}

void LinksTab::activateLinkByID(int theID)
{
	provinceMappingsGrid->activateLinkByID(theID);
}

void LinksTab::activateLinkByIndex(const int index)
{
	provinceMappingsGrid->activateLinkByIndex(index);
}

void LinksTab::activateTriangulationPairByID(const int theID) const
{
	triangulationPointGrid->activatePairByID(theID);
}

void LinksTab::activateTriangulationPairByIndex(int index)
{
	triangulationPointGrid->activatePairByIndex(index);
}

void LinksTab::refreshActiveLink()
{
	provinceMappingsGrid->refreshActiveLink();
}

void LinksTab::refreshActiveTriangulationPair()
{
	triangulationPointGrid->refreshActiveLink();
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
		case WXK_F7:
			stageAutogenerateMappings();
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
	if (triangulationPointGrid->getActiveLink())
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
	if (triangulationPointGrid->getActiveLink()) {
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR);
		eventListener->QueueEvent(evt->Clone());
	}
	// Do nothing unless working on active link. Don't want accidents here.
	else if (provinceMappingsGrid->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveUp() const
{	
	if (triangulationPointGrid->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_UP);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (provinceMappingsGrid->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveDown() const
{
	if (triangulationPointGrid->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (provinceMappingsGrid->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::moveActiveLinkUp()
{
	provinceMappingsGrid->moveActiveLinkUp();
	triangulationPointGrid->moveActiveLinkUp();
}

void LinksTab::moveActiveLinkDown()
{
	provinceMappingsGrid->moveActiveLinkDown();
	triangulationPointGrid->moveActiveLinkDown();
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

void LinksTab::stageAutogenerateMappings() const
{
	const auto* evt = new wxCommandEvent(wxEVT_AUTOGENERATE_MAPPINGS);
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
