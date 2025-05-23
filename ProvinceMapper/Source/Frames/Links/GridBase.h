#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);


class LinkBase;
class LinkMappingVersion;
class GridBase: public wxGrid
{
  public:
	GridBase(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);

	virtual void leftUp(const wxGridEvent& event) = 0;
	virtual void rightUp(wxGridEvent& event) = 0;
	void moveActiveLinkUp();
	void moveActiveLinkDown();

	void refreshActiveLink();

	[[nodiscard]] virtual const std::shared_ptr<LinkBase> getActiveLink() = 0;

  private:
	void onGridMotion(wxMouseEvent& event);

  protected:
	void focusOnActiveRow();

	std::shared_ptr<LinkMappingVersion> version;

	std::optional<int> activeRow;
	int lastClickedRow = 0;

	wxEvtHandler* eventListener = nullptr;
};
