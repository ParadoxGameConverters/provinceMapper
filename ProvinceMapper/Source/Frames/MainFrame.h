#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "PointWindow.h"
#include <wx/notebook.h>
#include "ImageTab.h"
#include "../PointMapper/PointMapper.h"

class MainFrame: public wxFrame
{
  public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	void initFrame();

  private:
	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);
	void onSupportUs(wxCommandEvent& event);
	void onPointPlaced(wxCommandEvent& event);
	void onUpdatePoint(wxCommandEvent& event);
	void onDeselectWorkingPoint(wxCommandEvent& event);
	void onChangeTab(wxCommandEvent& event);
	void onDeleteWorkingPoint(wxCommandEvent& event);
	void onExportPoints(wxCommandEvent& event);
	void onExportAdjustedMap(wxCommandEvent& event);

	PointMapper pointMapper;
	PointWindow* pointWindow = nullptr;
	wxNotebook* notebook = nullptr;
	wxFlexGridSizer* vbox = nullptr;
	ImageTab* imageTabFrom = nullptr;
	ImageTab* imageTabTo = nullptr;
};
