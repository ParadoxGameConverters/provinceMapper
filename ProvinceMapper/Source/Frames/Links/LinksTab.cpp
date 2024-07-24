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
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &LinksTab::leftUp, this);
	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &LinksTab::rightUp, this);
	Bind(wxEVT_UPDATE_NAME, &LinksTab::onUpdateComment, this);
	Bind(wxEVT_KEY_DOWN, &LinksTab::onKeyDown, this);

	wxStaticText* pairsTitle = new wxStaticText(this, wxID_ANY, "Triangulation Pairs", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	pairsTitle->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	triangulationPointGrid = new TriangulationPairsGrid(this, version);
	GetParent()->Layout();

	wxStaticText* linksTitle = new wxStaticText(this, wxID_ANY, "Province Links", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	linksTitle->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	theGrid = new ProvinceMappingsGrid(this, version);
	GetParent()->Layout();

	auto* gridBox = new wxBoxSizer(wxVERTICAL);
	gridBox->Add(pairsTitle, 0, wxALIGN_CENTER | wxALL, 10);
	gridBox->Add(triangulationPointGrid, 1, wxEXPAND | wxALL);

	gridBox->AddSpacer(20); // Visually separate the triangulation pairs grid from the province links grid.
	gridBox->Add(pairsTitle, 0, wxALIGN_CENTER | wxALL, 10);
	gridBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(gridBox);
	gridBox->Fit(this);
	triangulationPointGrid->ForceRefresh();
	theGrid->ForceRefresh();
}


void LinksTab::redraw()
{
	triangulationPointGrid->redraw();
	theGrid->redraw();
}

std::string LinksTab::triangulationPairToString(const std::shared_ptr<TriangulationPointPair>& pair)
{
	std::string name;

	const auto& sourcePoint = pair->getSourcePoint();
	if (sourcePoint)
	{
		wxString sourcePointStr = wxString::Format("(%d, %d)", sourcePoint->x, sourcePoint->y);
		name += std::string(sourcePointStr.mb_str());
	}

	name += " -> ";

	const auto& targetPoint = pair->getTargetPoint();
	if (targetPoint)
	{
		wxString targetPointStr = wxString::Format("(%d, %d)", targetPoint->x, targetPoint->y);
		name += std::string(targetPointStr.mb_str());
	}

	if (pair->getComment())
		name += " " + *pair->getComment();

	return name;
}

void LinksTab::leftUp(const wxGridEvent& event)
{
	if (event.GetId() == theGrid->GetId()) // TODO: REMOVE THIS IF BECAUSE THE GRID ARE BEING EXTRACTED TO SEPARATE FILES
	{
		theGrid->leftUp(event);
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
	theGrid->activateLinkByIndex(index);
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

void LinksTab::createTriangulationPair(int pairID)
{
	// We could just redraw the entire grid but that flickers. This is more complicated but cleaner on the eyes.

	// Where is this new row?
	auto rowCounter = 0;
	for (const auto& pair: *version->getTriangulationPointPairs())
	{
		if (pair->getID() == pairID)
		{
			triangulationPointGrid->InsertRows(rowCounter, 1, false);
			std::string name;
			name = triangulationPairToString(pair);

			triangulationPointGrid->SetCellValue(rowCounter, 0, name);

			activateTriangulationPairRowColor(rowCounter);
			activeTriangulationPair = pair;
			// If we have an active link, restore its color.
			if (activeTriangulationPointRow)
				restoreTriangulationPairRowColor(*activeTriangulationPointRow + 1); // We have a link inserted so we need to fix the following one.
			activeTriangulationPointRow = rowCounter;
			lastClickedTriangulationPairRow = rowCounter;
			// let's insert it.
			triangulationPointGrid->SetColMinimalWidth(0, 600);
			triangulationPointGrid->ForceRefresh();
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
		case WXK_F6: // TODO: maybe this should be swapped with F5?
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
	if (triangulationPointGrid->activeTriangulationPair)
	{
		triangulationPointGrid->stageAddComment();
	}
	else
	{
		theGrid->stageAddComment();
	}
}

void LinksTab::stageDeleteLink() const
{
	if (triangulationPointGrid->activeTriangulationPair) {
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR);
		eventListener->QueueEvent(evt->Clone());
	}
	// Do nothing unless working on active link. Don't want accidents here.
	else if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveUp() const
{	
	if (triangulationPointGrid->activeTriangulationPair)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_UP);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::stageMoveDown() const
{
	if (triangulationPointGrid->activeTriangulationPair)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_TRIANGULATION_PAIR_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
	else if (activeLink)
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
		eventListener->QueueEvent(evt->Clone());
	}
}

void LinksTab::moveActiveLinkUp()
{
	theGrid->moveActiveLinkUp();
}

void LinksTab::moveActiveLinkDown()
{
	theGrid->moveActiveLinkDown();
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
