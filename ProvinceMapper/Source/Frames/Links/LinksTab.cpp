#include "LinksTab.h"
#include "Provinces/Province.h"
#include "LinkMapper/LinkMappingVersion.h"

LinksTab::LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID):
	 wxNotebookPage(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), version(std::move(theVersion)), eventListener(parent), ID(theID)
{
	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->SetColLabelValue(0, version->getName());
	theGrid->SetColLabelAlignment(wxLEFT, wxCENTER);
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColLabelSize(20);

	theGrid->SetMinSize(wxSize(600, 900));
	GetParent()->Layout();

	auto* logBox = new wxBoxSizer(wxVERTICAL);
	logBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(logBox);
	logBox->Fit(this);
}

void LinksTab::redrawGrid() const
{
	auto rowCounter = 0;
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

	for (const auto& link: *version->getLinks())
	{
		auto bgColor = wxColour(240, 240, 240);
		std::string name;
		std::string comma;
		if (!link->getComment().empty())
		{
			name = link->getComment();
			bgColor = wxColour(150, 150, 150);
		}
		else
		{
			for (const auto& source: link->getSources())
			{
				name += comma;
				if (source->mapDataName.empty())
					name += "(No Name)";
				else
					name += source->mapDataName;
				comma = ", ";
			}
			name += " -> ";
			comma.clear();
			for (const auto& target: link->getTargets())
			{
				name += comma;
				if (target->mapDataName.empty())
					name += "(No Name)";
				else
					name += target->mapDataName;
				comma = ", ";
			}
		}
		theGrid->AppendRows(1, false);
		theGrid->SetRowSize(rowCounter, 20);
		theGrid->SetCellValue(rowCounter, 0, name);
		theGrid->SetCellAlignment(rowCounter, 0, wxCENTER, wxCENTER);
		theGrid->SetCellBackgroundColour(rowCounter, 0, bgColor);
		theGrid->SetReadOnly(rowCounter, 0);
		rowCounter++;
	}

	theGrid->EndBatch();
	theGrid->AutoSize();
	GetParent()->Layout();
	theGrid->Scroll(0, rowCounter - 1);
}
