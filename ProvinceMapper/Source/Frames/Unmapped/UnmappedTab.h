#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wx/grid.h>
#include <wx/notebook.h>

wxDECLARE_EVENT(wxEVT_PROVINCE_CENTER_MAP, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_UPDATE_PROVINCE_COUNT, wxCommandEvent);

enum class ImageTabSelector;
class LinkMappingVersion;
class Province;
class LinkMapping;
class UnmappedTab final: public wxNotebookPage
{
  public:
	UnmappedTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, ImageTabSelector theSelector);
	void redrawGrid();
	void setVersion(const std::shared_ptr<LinkMappingVersion>& theVersion) { version = theVersion; }
	void removeProvince(const std::string& ID);
	void addProvince(const std::string& ID);
	void setExcludeWaterProvinces(bool excludeWaterProvinces);
	void setExcludeImpassables(bool excludeImpassables);

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
	void stageUpdateProvinceCount() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;
	void focusOnRow(int row);

	[[nodiscard]] const std::vector<std::shared_ptr<Province>> getRelevantProvinces() const;

	wxGrid* theGrid = nullptr;
	ImageTabSelector selector;

	std::shared_ptr<LinkMappingVersion> version;
	std::map<std::string, int> provinceRows; // province/row
	bool excludeWaterProvinces = false;
	bool excludeImpassables = false;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
