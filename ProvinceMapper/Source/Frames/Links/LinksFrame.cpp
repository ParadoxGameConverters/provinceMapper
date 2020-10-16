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

	auto counter = 0;
	for (const auto& version: versions)
	{
		auto* newTab = new LinksTab(notebook, version, counter);
		newTab->SetBackgroundColour(wxColour(255, 245, 245));
		notebook->AddPage(newTab, version->getName());
		versionToPage.insert(std::pair(version->getName(), counter));
		pages.insert(std::pair(counter, newTab));
		if (version->getName() == activeVersion->getName())
			activePage = std::pair(counter, newTab);
		newTab->redrawGrid();
		++counter;
	}
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

void LinksFrame::deactivateLink()
{
	if (activePage.second)
		activePage.second->deactivateLink();
}

void LinksFrame::activateLinkByID(const int ID)
{
	if (activePage.second)
		activePage.second->activateLinkByID(ID);
}
