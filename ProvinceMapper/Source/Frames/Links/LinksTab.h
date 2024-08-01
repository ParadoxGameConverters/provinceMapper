#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>
#include <wx/grid.h>
#include <wx/notebook.h>


wxDECLARE_EVENT(wxEVT_DELETE_ACTIVE_LINK_OR_TRIANGULATION_PAIR, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_LEFT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOVE_ACTIVE_VERSION_RIGHT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVE_LINKS, wxCommandEvent);


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
	void deactivateTriangulationPair();
	void activateLinkByID(int theID);
	void activateLinkByIndex(int index);
	void activateTriangulationPairByIndex(int index);
	void refreshActiveLink();
	void refreshActiveTriangulationPair();
	void createLink(int linkID);
	void createTriangulationPair(int pairID);
	void moveActiveLinkUp();
	void moveActiveLinkDown();

  private:
	void onKeyDown(wxKeyEvent& event);	// TODO: add triangulation pairs support

	void stageAddComment();
	void stageDeleteLink() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageAddTriangulationPair() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

	int lastClickedTriangulationPairRow = 0; // TODO: move this to TriangulationPairsGrid

	TriangulationPairsGrid* triangulationPointGrid = nullptr;
	ProvinceMappingsGrid* provinceMappingsGrid = nullptr;
	std::shared_ptr<LinkMappingVersion> version;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
