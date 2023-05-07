#include "UnmappedFrame.h"
#include "Configuration/Configuration.h"
#include "Frames/Images/ImageCanvas.h"
#include "Frames/Links/LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "UnmappedTab.h"

UnmappedFrame::UnmappedFrame(wxWindow* parent,
	 const wxPoint& position,
	 const wxSize& size,
	 const std::shared_ptr<LinkMappingVersion>& activeVersion,
	 std::shared_ptr<Configuration> theConfiguration):
	 wxFrame(parent, wxID_ANY, "Unmapped Provinces", position, size, wxDEFAULT_FRAME_STYLE),
	 configuration(std::move(theConfiguration)), eventHandler(parent)
{
	Bind(wxEVT_SIZE, &UnmappedFrame::onResize, this);
	Bind(wxEVT_CLOSE_WINDOW, &UnmappedFrame::onClose, this);
	Bind(wxEVT_KEY_DOWN, &UnmappedFrame::onKeyDown, this);
	Bind(wxEVT_MOVE, &UnmappedFrame::onMove, this);
	Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &UnmappedFrame::onChangeTab, this);
	Bind(wxEVT_UPDATE_PROVINCE_COUNT, &UnmappedFrame::onUpdateProvinceCount, this);

	auto* sizer = new wxBoxSizer(wxVERTICAL);

	notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	notebook->Bind(wxEVT_KEY_DOWN, &UnmappedFrame::onKeyDown, this);

	sources = new UnmappedTab(notebook, activeVersion, ImageTabSelector::SOURCE);
	notebook->AddPage(sources, "Source Provinces", false);
	sources->redrawGrid();
	targets = new UnmappedTab(notebook, activeVersion, ImageTabSelector::TARGET);
	notebook->AddPage(targets, "Target Provinces", false);
	targets->redrawGrid();

	excludeWaterProvincesCheckbox = new wxCheckBox(this, wxID_ANY, "Exclude water provinces");
	excludeWaterProvincesCheckbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (excludeWaterProvincesCheckbox->GetValue())
		{
			sources->setExcludeWaterProvinces(true);
			targets->setExcludeWaterProvinces(true);
		}
		else
		{
			sources->setExcludeWaterProvinces(false);
			targets->setExcludeWaterProvinces(false);
		}
	});
	sizer->Add(excludeWaterProvincesCheckbox);

	excludeImpassablesCheckbox = new wxCheckBox(this, wxID_ANY, "Exclude impassables");
	excludeImpassablesCheckbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
		if (excludeImpassablesCheckbox->GetValue())
		{
			sources->setExcludeImpassables(true);
			targets->setExcludeImpassables(true);
		}
		else
		{
			sources->setExcludeImpassables(false);
			targets->setExcludeImpassables(false);
		}
	});
	sizer->Add(excludeImpassablesCheckbox);

	sizer->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL, 1));
	this->SetSizer(sizer);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void UnmappedFrame::onResize(wxSizeEvent& event)
{
	const auto size = event.GetSize();
	if (!IsMaximized())
	{
		configuration->setUnmappedFrameSize(size.x, size.y);
		configuration->save();
	}
	notebook->SetVirtualSize(event.GetSize());
	notebook->Layout();
	event.Skip();
}

void UnmappedFrame::onClose(wxCloseEvent& event)
{
	configuration->setUnmappedFrameOn(false);
	configuration->save();
	Hide();
}

void UnmappedFrame::setVersion(const std::shared_ptr<LinkMappingVersion>& version) const
{
	sources->setVersion(version);
	targets->setVersion(version);

	sources->redrawGrid();
	targets->redrawGrid();
}

void UnmappedFrame::onKeyDown(wxKeyEvent& event)
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

void UnmappedFrame::stageMoveVersionLeft() const
{
	const auto evt = wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventHandler->AddPendingEvent(evt);
}

void UnmappedFrame::stageMoveVersionRight() const
{
	const auto evt = wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventHandler->AddPendingEvent(evt);
}

void UnmappedFrame::onMove(wxMoveEvent& event)
{
	if (IsMaximized())
	{
		configuration->setUnmappedFrameMaximized(true);
	}
	else
	{
		const auto position = GetPosition();
		configuration->setUnmappedFramePos(position.x, position.y);
		configuration->setUnmappedFrameMaximized(false);
	}
	configuration->save();
	event.Skip();
}

void UnmappedFrame::onChangeTab(wxBookCtrlEvent& event)
{
	// Capture this event so it doesn't get mistaken for change active version event from LinksFrame.
}

void UnmappedFrame::removeProvince(const std::string& ID, const bool sourceTab)
{
	if (sourceTab)
		sources->removeProvince(ID);
	else
		targets->removeProvince(ID);
}

void UnmappedFrame::addProvince(const std::string& ID, const bool sourceTab)
{
	if (sourceTab)
		sources->addProvince(ID);
	else
		targets->addProvince(ID);
}

void UnmappedFrame::onUpdateProvinceCount(const wxCommandEvent& event)
{
	if (event.GetId() == 0)
		notebook->SetPageText(0, "Source Provinces (" + std::to_string(event.GetInt()) + ")");
	else
		notebook->SetPageText(1, "Target Provinces (" + std::to_string(event.GetInt()) + ")");
}

void UnmappedFrame::refreshList() const
{
	sources->redrawGrid();
	targets->redrawGrid();
}
