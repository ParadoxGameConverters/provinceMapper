#include "LinksFrame.h"

#include "Log.h"
#include "../../Provinces/Province.h"
#include "../../LinkMapper/LinkMappingVersion.h"

LinksFrame::LinksFrame(wxWindow* parent,
	 std::vector<std::shared_ptr<LinkMappingVersion>> theVersions,
	 std::shared_ptr<LinkMappingVersion> theActiveVersion):
	 wxFrame(parent, wxID_ANY, "Links", wxDefaultPosition, wxSize(600, 900), wxDEFAULT_FRAME_STYLE),
	 eventListener(parent), versions(std::move(theVersions)), activeVersion(std::move(theActiveVersion))
{}

void LinksFrame::initFrame()
{
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 900));

	auto counter = 0;
	for (const auto& version: versions)
	{
		Log(LogLevel::Debug) << "Inserting version: " << version->getName();
		LinksTab* newTab = new LinksTab(notebook, version, counter);
		newTab->SetBackgroundColour(wxColour(255, 245, 245));
		notebook->AddPage(newTab, version->getName());
		pages.insert(std::pair(counter, newTab));
		if (version->getName() == activeVersion->getName())
			activePage = std::pair(counter, newTab);
		newTab->redrawGrid();
		++counter;
	}
	vbox->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL, 1));
	this->SetSizer(vbox);
	this->Centre();

	Bind(wxEVT_SIZE, &LinksFrame::onResize, this);
}

void LinksFrame::onResize(wxSizeEvent& event)
{
	// layout everything in the dialog
	notebook->SetVirtualSize(event.GetSize());
	notebook->Layout();
	event.Skip();
}
