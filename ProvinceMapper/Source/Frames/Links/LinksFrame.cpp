#include "LinksFrame.h"
#include "Configuration/Configuration.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

LinksFrame::LinksFrame(wxWindow* parent,
	 const wxPoint& position,
	 const wxSize& size,
	 const std::vector<std::shared_ptr<LinkMappingVersion>>& versions,
	 const std::shared_ptr<LinkMappingVersion>& activeVersion,
	 std::shared_ptr<Configuration> theConfiguration):
	 wxFrame(parent, wxID_ANY, "Links", position, size, wxDEFAULT_FRAME_STYLE),
	 configuration(std::move(theConfiguration)), eventHandler(parent)
{
	Bind(wxEVT_SIZE, &LinksFrame::onResize, this);
	Bind(wxEVT_CLOSE_WINDOW, &LinksFrame::onClose, this);
	Bind(wxEVT_KEY_DOWN, &LinksFrame::onKeyDown, this);
	Bind(wxEVT_MOVE, &LinksFrame::onMove, this);

	auto* sizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	notebook->Bind(wxEVT_KEY_DOWN, &LinksFrame::onKeyDown, this);

	for (const auto& version: versions)
	{
		auto* newTab = new LinksTab(notebook, version);
		notebook->AddPage(newTab, version->getName(), false);
		versionIDs.emplace_back(version->getID());
		pages.insert(std::pair(version->getID(), newTab));
		if (version->getName() == activeVersion->getName())
			activePage = newTab;
		newTab->redrawGrid();
	}
	notebook->ChangeSelection(0); // silently swap to first page.
	sizer->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL, 1));
	this->SetSizer(sizer);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void LinksFrame::onResize(wxSizeEvent& event)
{
	const auto size = event.GetSize();
	if (!IsMaximized())
	{
		configuration->setLinksFrameSize(size.x, size.y);
		configuration->save();
	}
	notebook->SetVirtualSize(event.GetSize());
	notebook->Layout();
	event.Skip();
}

void LinksFrame::onClose(const wxCloseEvent& event)
{
	// We need to kill the app.
	eventHandler->QueueEvent(event.Clone());
}

void LinksFrame::deactivateLink() const
{
	if (activePage)
		activePage->deactivateLink();
}

void LinksFrame::activateLinkByID(const int ID) const
{
	if (activePage)
		activePage->activateLinkByID(ID);
}

void LinksFrame::activateLinkByIndex(const int index) const
{
	if (activePage)
		activePage->activateLinkByIndex(index);
}

void LinksFrame::refreshActiveLink() const
{
	if (activePage)
		activePage->refreshActiveLink();
}

void LinksFrame::createLink(const int linkID) const
{
	if (activePage)
		activePage->createLink(linkID);
}

void LinksFrame::createTriangulationPair(const int pairID) const
{
	if (activePage)
	{
		activePage->createTriangulationPair(pairID);
	}
}

void LinksFrame::addVersion(const std::shared_ptr<LinkMappingVersion>& version)
{
	auto* newTab = new LinksTab(notebook, version);
	notebook->InsertPage(0, newTab, version->getName(), false); // don't swap!
	notebook->ChangeSelection(0);											// silently swap!
	versionIDs.insert(versionIDs.begin(), version->getID());
	pages.insert(std::pair(version->getID(), newTab));
	activePage = newTab;
	newTab->redrawGrid();
}

void LinksFrame::deleteActiveAndSwapToVersion(const std::shared_ptr<LinkMappingVersion>& version)
{
	const auto originalPage = static_cast<size_t>(notebook->GetSelection());
	const auto originalPages = versionIDs.size();
	setVersion(version);
	if (originalPages != versionIDs.size())
	{
		notebook->DeletePage(originalPage + 1);
		versionIDs.erase(versionIDs.begin() + originalPage + 1);
	}
	else
	{
		notebook->DeletePage(originalPage);
		versionIDs.erase(versionIDs.begin() + originalPage);
	}
}

void LinksFrame::setVersion(const std::shared_ptr<LinkMappingVersion>& version)
{
	// Do we have this version loaded?
	const auto& idItr = std::ranges::find(versionIDs, version->getID());
	if (idItr == versionIDs.end())
	{
		// this is a new version.
		addVersion(version);
	}
	else
	{
		// we have it at this page.
		const auto page = static_cast<int>(std::distance(versionIDs.begin(), idItr));
		notebook->ChangeSelection(page);
		activePage = pages[version->getID()];
	}
}

void LinksFrame::updateActiveVersionName(const std::string& theName) const
{
	const auto page = notebook->GetSelection();
	if (page != wxNOT_FOUND)
	{
		notebook->SetPageText(page, theName);
	}
}

void LinksFrame::moveActiveLinkUp() const
{
	if (activePage)
		activePage->moveActiveLinkUp();
}

void LinksFrame::moveActiveLinkDown() const
{
	if (activePage)
		activePage->moveActiveLinkDown();
}

void LinksFrame::moveActiveVersionLeft()
{
	if (activePage)
	{
		const auto originalPage = static_cast<size_t>(notebook->GetSelection());
		const auto name = notebook->GetPageText(originalPage);
		if (originalPage > 0)
		{
			notebook->InsertPage(originalPage - 1, activePage, name, false);
			std::swap(versionIDs[originalPage], versionIDs[originalPage - 1]);
			notebook->RemovePage(originalPage + 1);
			notebook->SetSelection(originalPage - 1);
		}
	}
}

void LinksFrame::moveActiveVersionRight()
{
	if (activePage)
	{
		const auto originalPage = static_cast<size_t>(notebook->GetSelection());
		const auto name = notebook->GetPageText(originalPage);
		if (originalPage < versionIDs.size() - 1)
		{
			notebook->InsertPage(originalPage + 2, activePage, name, false);
			std::swap(versionIDs[originalPage], versionIDs[originalPage + 1]);
			notebook->RemovePage(originalPage);
			notebook->SetSelection(originalPage + 1);
		}
	}
}

void LinksFrame::onKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
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

void LinksFrame::stageMoveVersionLeft() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventHandler->QueueEvent(evt->Clone());
}

void LinksFrame::stageMoveVersionRight() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventHandler->QueueEvent(evt->Clone());
}

void LinksFrame::onMove(wxMoveEvent& event)
{
	if (IsMaximized())
	{
		configuration->setLinksFrameMaximized(true);
	}
	else
	{
		const auto position = GetPosition();
		configuration->setLinksFramePos(position.x, position.y);
		configuration->setLinksFrameMaximized(false);
	}
	configuration->save();
	event.Skip();
}
