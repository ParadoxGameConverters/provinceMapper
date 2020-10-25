#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class StatusBar: public wxFrame
{
  public:
	StatusBar(wxWindow* parent);

	void setSourceZoom(int zoomLevel) const;
	void setTargetZoom(int zoomLevel) const;

  private:
	wxTextCtrl* sourceZoomField = nullptr;
	wxTextCtrl* targetZoomField = nullptr;

	void onZoomChanged(wxCommandEvent& evt);

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
