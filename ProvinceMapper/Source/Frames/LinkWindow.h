#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/grid.h>
#include "ImageBox.h"

wxDECLARE_EVENT(wxEVT_CHANGE_TAB, wxCommandEvent);

class LinkWindow: public wxWindow
{
  public:
	LinkWindow(wxWindow* parent);
	void redrawGrid() const;

  private:
	wxGrid* theGrid = nullptr;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
