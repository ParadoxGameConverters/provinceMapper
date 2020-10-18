#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>

wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DEACTIVATE_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);

class LinkMappingVersion;
class LinkMapping;
class LinksTab: public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, int theID);
	void redrawGrid();

	void deactivateLink();
	void activateLinkByID(int theID);
	void activateLinkByIndex(int index);
	void refreshActiveLink();
	void createLink(int linkID);

  private:
	void rightUp(wxGridEvent& event);
	void leftUp(wxGridEvent& event);
	void onGridMotion(wxMouseEvent& event);
	void onUpdateComment(wxCommandEvent& event);
	void onKeyDown(wxKeyEvent& event);

	void restoreRowColor(int row) const;
	void activateRowColor(int row) const;
	void stageAddComment();
	void stageDeleteLink() const;
	
	int ID = 0;
	int lastClickedRow = 0;

	wxGrid* theGrid = nullptr;
	std::optional<int> activeRow;
	std::shared_ptr<LinkMapping> activeLink;
	std::shared_ptr<LinkMappingVersion> version;

	void focusOnActiveRow();
	static std::string linkToString(const std::shared_ptr<LinkMapping>& link);

  protected:
	wxEvtHandler* eventListener = nullptr;
};
