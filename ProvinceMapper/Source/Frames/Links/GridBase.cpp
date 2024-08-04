#include "GridBase.h"
#include "LinkMapper/LinkBase.h"


GridBase::GridBase(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion):
	 wxGrid(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxEXPAND), version(theVersion), eventListener(parent)
{
	Bind(wxEVT_GRID_CELL_LEFT_CLICK, &GridBase::leftUp, this);
	Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &GridBase::rightUp, this);

	CreateGrid(0, 1, wxGrid::wxGridSelectCells);
	EnableEditing(false);
	HideCellEditControl();
	HideRowLabels();
	HideColLabels();
	SetScrollRate(0, 10);
	SetColMinimalAcceptableWidth(600);
	GetGridWindow()->Bind(wxEVT_MOTION, &GridBase::onGridMotion, this);
	SetColMinimalWidth(0, 600);
}


void GridBase::onGridMotion(wxMouseEvent& event)
{
	// We do NOT want to select cells, alter their size or similar nonsense.
	// Thus, we're preventing mouse motion events to propagate by not processing them.
}


void GridBase::focusOnActiveRow()
{
	const auto cellCoords = CellToRect(*activeRow, 0);				// these would be virtual coords, not logical ones.
	const auto units = cellCoords.y / 20;								// pixels into scroll units, 20 is our scroll rate defined in constructor.
	const auto scrollPageSize = GetScrollPageSize(wxVERTICAL);	// this is how much "scrolls" a pageful of cells scrolls.
	const auto offset = wxPoint(0, units - scrollPageSize / 2); // position ourselves at our cell, minus half a screen of scrolls.
	Scroll(offset);															// and shoo.
	ForceRefresh();
}


void GridBase::moveActiveLinkUp()
{
	if (activeLink && activeRow && *activeRow > 0)
	{
		const auto text = GetCellValue(*activeRow, 0);
		const auto color = GetCellBackgroundColour(*activeRow, 0);
		DeleteRows(*activeRow, 1, false);
		--*activeRow;
		InsertRows(*activeRow, 1, false);
		SetCellValue(*activeRow, 0, text);
		SetCellBackgroundColour(*activeRow, 0, color);
	}
}


void GridBase::moveActiveLinkDown()
{
	if (activeLink && activeRow && *activeRow < GetNumberRows() - 1)
	{
		const auto text = GetCellValue(*activeRow, 0);
		const auto color = GetCellBackgroundColour(*activeRow, 0);
		DeleteRows(*activeRow, 1, false);
		++*activeRow;
		InsertRows(*activeRow, 1, false);
		SetCellValue(*activeRow, 0, text);
		SetCellBackgroundColour(*activeRow, 0, color);
	}
}

void GridBase::refreshActiveLink()
{
	// this is called when we're toggling a province within the active link

	if (activeRow && activeLink)
	{
		SetCellValue(*activeRow, 0, activeLink->toRowString());
	}
}