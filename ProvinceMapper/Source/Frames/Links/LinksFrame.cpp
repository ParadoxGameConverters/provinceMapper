#include "LinksFrame.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

LinksFrame::LinksFrame(wxWindow* parent,
	 const std::vector<std::shared_ptr<LinkMappingVersion>>& versions,
	 const std::shared_ptr<LinkMappingVersion>& activeVersion):
	 wxFrame(parent, wxID_ANY, "Links", wxDefaultPosition, wxSize(600, 900), wxDEFAULT_FRAME_STYLE),
	 eventHandler(parent)

{
	Bind(wxEVT_SIZE, &LinksFrame::onResize, this);
	Bind(wxEVT_CLOSE_WINDOW, &LinksFrame::onClose, this);

	auto* sizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 900));

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
	this->Centre();

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void LinksFrame::onResize(wxSizeEvent& event)
{
	notebook->SetVirtualSize(event.GetSize());
	notebook->Layout();
	event.Skip();
}

void LinksFrame::onClose(wxCloseEvent& event)
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
	const auto& idItr = std::find(versionIDs.begin(), versionIDs.end(), version->getID());
	if (idItr == versionIDs.end())
	{
		// this is a new version.
		addVersion(version);
	}
	else
	{
		// we have it at this page.
		const auto page = static_cast<int>(std::distance(versionIDs.begin(), idItr));
		Log(LogLevel::Debug) << "changing active page to : " << page;
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
