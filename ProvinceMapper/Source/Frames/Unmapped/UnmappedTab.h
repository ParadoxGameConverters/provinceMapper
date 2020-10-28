#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/grid.h>
#include <wx/notebook.h>

wxDECLARE_EVENT(wxEVT_PROVINCE_CENTER_MAP, wxCommandEvent);

enum class ImageTabSelector;
class LinkMappingVersion;
class Province;
class LinkMapping;
class UnmappedTab: public wxNotebookPage
{
  public:
	UnmappedTab(wxWindow* parent, std::shared_ptr<LinkMappingVersion> theVersion, ImageTabSelector theSelector);
	void redrawGrid() const;
	void setVersion(const std::shared_ptr<LinkMappingVersion>& theVersion) { version = theVersion; }

  private:
	void onKeyDown(wxKeyEvent& event);
	void leftUp(wxGridEvent& event);
	void onGridMotion(wxMouseEvent& event);

	void stageAddComment() const;
	void stageDeleteLink() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

	[[nodiscard]] const std::vector<std::shared_ptr<Province>>& getRelevantProvinces() const;

	wxGrid* theGrid = nullptr;
	ImageTabSelector selector;

	std::shared_ptr<LinkMappingVersion> version;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
