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
wxDECLARE_EVENT(wxEVT_DEACTIVATE_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_INDEX, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_CENTER_MAP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_UP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_LINK_DOWN, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_LINK, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);

class LinkMappingVersion;
class LinkMapping;
class TriangulationPointPair;
class LinksTab final : public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void redrawTriangulationPairsGrid();
	void redrawGrid();

	void restoreTriangulationPairRowColor(int pairRow) const;

	void activateTriangulationPairRowColor(int row) const;

	void deactivateLink();
	void activateLinkByID(int theID);
	void activateLinkByIndex(int index);
	void refreshActiveLink();
	void createLink(int linkID);
	void createTriangulationPair(int pairID);
	void moveActiveLinkUp(); // TODO: add triangulation pairs support
	void moveActiveLinkDown(); // TODO: add triangulation pairs support

  private:
	void rightUp(wxGridEvent& event); 
	void leftUp(const wxGridEvent& event);
	void onGridMotion(wxMouseEvent& event); // TODO: add triangulation pairs support
	void onUpdateComment(const wxCommandEvent& event); // TODO: add triangulation pairs support
	void onKeyDown(wxKeyEvent& event);	// TODO: add triangulation pairs support

	void restoreLinkRowColor(int row) const;
	void activateLinkRowColor(int row) const;
	void stageAddComment();
	void stageDeleteLink() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageAddTriangulationPair() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

	int lastClickedTriangulationPairRow = 0;
	int lastClickedRow = 0;

	wxGrid* triangulationPointGrid = nullptr;
	std::optional<int> activeTriangulationPointRow;
	std::shared_ptr<TriangulationPointPair> activeTriangulationPair;
	void focusOnActiveTriangulationPairRow(); // TODO: check if needed

	wxGrid* theGrid = nullptr;
	std::optional<int> activeRow;
	std::shared_ptr<LinkMapping> activeLink;
	std::shared_ptr<LinkMappingVersion> version;

	void focusOnActiveRow();
	static std::string linkToString(const std::shared_ptr<LinkMapping>& link);

	std::string triangulationPairToString(const std::shared_ptr<TriangulationPointPair>& pair);

	void linksGridLeftUp(const wxGridEvent& event);

	void triangulationPairsGridLeftUp(const wxGridEvent& event);

  protected:
	wxEvtHandler* eventListener = nullptr;
};
