#include "LinkWindow.h"

wxDEFINE_EVENT(wxEVT_CHANGE_TAB, wxCommandEvent);

LinkWindow::LinkWindow(wxWindow* parent): wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	eventListener = parent;

	// Pointwindow displays the points we get from PointMapper.
	
	theGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
	theGrid->CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	theGrid->HideCellEditControl();
	theGrid->HideRowLabels();
	theGrid->SetColLabelValue(0, "Link");
	theGrid->SetColLabelAlignment(wxLEFT, wxCENTER);
	theGrid->SetScrollRate(0, 20);
	theGrid->SetColLabelSize(20);

	theGrid->AutoSize();
	GetParent()->Layout();

	wxBoxSizer* logBox = new wxBoxSizer(wxVERTICAL);
	logBox->Add(theGrid, wxSizerFlags(1).Expand());
	SetSizer(logBox);
	logBox->Fit(this);
}

void LinkWindow::redrawGrid() const
{
	auto rowCounter = 0;
	theGrid->BeginBatch();
	theGrid->DeleteRows(0, theGrid->GetNumberRows());

	theGrid->EndBatch();
	theGrid->AutoSize();
	GetParent()->Layout();
	theGrid->Scroll(0, rowCounter - 1);
}
