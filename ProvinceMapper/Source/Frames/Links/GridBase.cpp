#include "GridBase.h"





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