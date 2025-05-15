#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wx/grid.h>
#include <wx/notebook.h>


enum class ImageTabSelector;
class LinkMappingVersion;
class Province;
class LinkMapping;
class SearchTab final: public wxNotebookPage
{
  public:
	SearchTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion);
	void redrawGrid();
	void setVersion(const std::shared_ptr<LinkMappingVersion>& theVersion) { version = theVersion; }
	void removeProvince(const std::string& ID);
	void addProvince(const std::string& ID);
	void setSearchString(const std::string& searchStr);

  private:
	void onKeyDown(wxKeyEvent& event);
	void leftUp(const wxGridEvent& event);
	void onGridMotion(wxMouseEvent& event);

	void stageAddComment() const;
	void stageDeleteLinkOrTriangulationPair() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageAddTriangulationPair() const;
	void stageAutogenerateMappings() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;
	void focusOnRow(int row);

	[[nodiscard]] const std::vector<std::shared_ptr<LinkMapping>> getRelevantLinks() const;

	wxGrid* theGrid = nullptr;

	std::shared_ptr<LinkMappingVersion> version;
	std::map<int, int> linkRows; // link ID/row
	std::string searchString;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
