#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "LinkWindow.h"
#include <wx/notebook.h>
#include "ImageTab.h"
#include "../Definitions/Definitions.h"
#include "../LinkMapper/LinkMapper.h"

class MainFrame: public wxFrame
{
  public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	void initFrame();

  private:
	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);
	void onSupportUs(wxCommandEvent& event);
	void onChangeTab(wxCommandEvent& event);

	LinkWindow* linkWindow = nullptr;
	wxNotebook* notebook = nullptr;
	wxFlexGridSizer* vbox = nullptr;
	ImageTab* imageTabFrom = nullptr;
	ImageTab* imageTabTo = nullptr;

	Definitions sourceDefs;
	Definitions targetDefs;
	LinkMapper linkMapper;
};
