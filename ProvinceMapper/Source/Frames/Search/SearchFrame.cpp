#include "SearchFrame.h"
#include "Configuration/Configuration.h"
#include "Frames/Images/ImageCanvas.h"
#include "Frames/Links/LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "SearchTab.h"

SearchFrame::SearchFrame(wxWindow* parent,
	 const wxPoint& position,
	 const wxSize& size,
	 const std::shared_ptr<LinkMappingVersion>& activeVersion,
	 std::shared_ptr<Configuration> theConfiguration):
	 wxFrame(parent, wxID_ANY, "Search by province ID or name", position, size, wxDEFAULT_FRAME_STYLE),
	 configuration(std::move(theConfiguration)), eventHandler(parent)
{
	Bind(wxEVT_SIZE, &SearchFrame::onResize, this);
	Bind(wxEVT_CLOSE_WINDOW, &SearchFrame::onClose, this);
	Bind(wxEVT_KEY_DOWN, &SearchFrame::onKeyDown, this);
	Bind(wxEVT_MOVE, &SearchFrame::onMove, this);

	auto* sizer = new wxBoxSizer(wxVERTICAL);

	notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	notebook->Bind(wxEVT_KEY_DOWN, &SearchFrame::onKeyDown, this);

	searchTab = new SearchTab(notebook, activeVersion);
	notebook->AddPage(searchTab, "Found links", false);
	searchTab->redrawGrid();

	searchCtrl = new wxSearchCtrl(this, wxID_ANY);
	searchCtrl->Bind(wxEVT_SEARCH, [this](wxCommandEvent& event) {
		const auto searchStr = std::string(event.GetString().mb_str());
		searchTab->setSearchString(searchStr);
	});
	searchCtrl->Bind(wxEVT_SEARCH_CANCEL, [this](wxCommandEvent& event) {
		searchCtrl->SetValue("");
		searchTab->setSearchString("");
	});
	searchCtrl->ShowCancelButton(true);
	searchCtrl->ShowSearchButton(true);
	sizer->Add(searchCtrl);

	sizer->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL, 1));
	this->SetSizer(sizer);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void SearchFrame::onResize(wxSizeEvent& event)
{
	const auto size = event.GetSize();
	if (!IsMaximized())
	{
		configuration->setSearchFrameSize(size.x, size.y);
		configuration->save();
	}
	notebook->SetVirtualSize(event.GetSize());
	notebook->Layout();
	event.Skip();
}

void SearchFrame::onClose(wxCloseEvent& event)
{
	configuration->setSearchFrameOn(false);
	configuration->save();
	Hide();
}

void SearchFrame::setVersion(const std::shared_ptr<LinkMappingVersion>& version) const
{
	searchTab->setVersion(version);
	searchTab->redrawGrid();
}

void SearchFrame::onKeyDown(wxKeyEvent& event)
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

void SearchFrame::stageMoveVersionLeft() const
{
	const auto evt = wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventHandler->AddPendingEvent(evt);
}

void SearchFrame::stageMoveVersionRight() const
{
	const auto evt = wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventHandler->AddPendingEvent(evt);
}

void SearchFrame::onMove(wxMoveEvent& event)
{
	if (IsMaximized())
	{
		configuration->setSearchFrameMaximized(true);
	}
	else
	{
		const auto position = GetPosition();
		configuration->setSearchFramePos(position.x, position.y);
		configuration->setSearchFrameMaximized(false);
	}
	configuration->save();
	event.Skip();
}

void SearchFrame::onChangeTab(wxBookCtrlEvent& event)
{
	// Capture this event so it doesn't get mistaken for change active version event from LinksFrame.
}

void SearchFrame::refreshList() const
{
	searchTab->redrawGrid();
}
