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
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);

class LinkMappingVersion;
class LinkMapping;
class LinksTab final : public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void redrawGrid();

	void deactivateLink();
	void activateLinkByID(int theID);
	void activateLinkByIndex(int index);
	void refreshActiveLink();
	void createLink(int linkID);
	void moveActiveLinkUp();
	void moveActiveLinkDown();

  private:
	void rightUp(wxGridEvent& event);
	void leftUp(const wxGridEvent& event);
	void onGridMotion(wxMouseEvent& event);
	void onUpdateComment(const wxCommandEvent& event);
	void onKeyDown(wxKeyEvent& event);

	void restoreRowColor(int row) const;
	void activateRowColor(int row) const;
	void stageAddComment();
	void stageDeleteLink() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

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
