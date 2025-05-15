#pragma once  
#include <wx/wxprec.h>  
#ifndef WX_PRECOMP  
#include <wx/wx.h>  
#endif  
#include <wx/notebook.h>  
#include <wx/srchctrl.h>

class SearchTab;
class wxGrid;
class Configuration;  
class LinkMappingVersion;  
class UnmappedTab;  
class SearchFrame final: public wxFrame  
{  
  public:  
	SearchFrame(wxWindow* parent,  
   const wxPoint& position,  
   const wxSize& size,  
   const std::shared_ptr<LinkMappingVersion>& activeVersion,  
   std::shared_ptr<Configuration> theConfiguration);  

	void setVersion(const std::shared_ptr<LinkMappingVersion>& version) const;  
	void removeProvince(const std::string& ID, bool sourceTab);  
	void addProvince(const std::string& ID, bool sourceTab);  
	void refreshList() const;  

  private:
	void onKeyDown(wxKeyEvent& event);  
	void onResize(wxSizeEvent& event);  
	void onClose(wxCloseEvent& event);  
	void onMove(wxMoveEvent& event);
	void onChangeTab(wxBookCtrlEvent& event);
	void onUpdateProvinceCount(const wxCommandEvent& event);

	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;
	
	wxNotebook* notebook = nullptr;
	SearchTab* searchTab;
	wxSearchCtrl* searchCtrl;

	wxGrid* theGrid = nullptr;

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
