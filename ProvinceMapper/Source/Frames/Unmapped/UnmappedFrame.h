#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/notebook.h>

class Configuration;
class LinkMappingVersion;
class UnmappedTab;
class UnmappedFrame final : public wxFrame
{
  public:
	UnmappedFrame(wxWindow* parent,
		 const wxPoint& position,
		 const wxSize& size,
		 const std::shared_ptr<LinkMappingVersion>& activeVersion,
		 std::shared_ptr<Configuration> theConfiguration);

	void setVersion(const std::shared_ptr<LinkMappingVersion>& version) const;
	void removeProvince(int ID, bool sourceTab);
	void addProvince(int ID, bool sourceTab);
	void refreshList() const;

  private:
	wxNotebook* notebook = nullptr;

	void onKeyDown(wxKeyEvent& event);
	void onResize(wxSizeEvent& evt);
	void onClose(wxCloseEvent& event);
	void onMove(wxMoveEvent& event);
	void onChangeTab(wxBookCtrlEvent& event);
	void onUpdateProvinceCount(wxCommandEvent& event);

	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;

	UnmappedTab* sources;
	UnmappedTab* targets;

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
