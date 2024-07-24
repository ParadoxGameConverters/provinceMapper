#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);


class LinkMappingVersion;
class LinkMapping;
class TriangulationPointPair;
class TriangulationPairsGrid;
class ProvinceMappingsGrid;
class LinksTab final: public wxNotebookPage
{
  public:
	LinksTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void redraw();

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

	TriangulationPairsGrid* triangulationPointGrid = nullptr;

	ProvinceMappingsGrid* theGrid = nullptr;
	std::shared_ptr<LinkMappingVersion> version;

	std::string triangulationPairToString(const std::shared_ptr<TriangulationPointPair>& pair);

	void triangulationPairsGridLeftUp(const wxGridEvent& event);

  protected:
	wxEvtHandler* eventListener = nullptr;
};
