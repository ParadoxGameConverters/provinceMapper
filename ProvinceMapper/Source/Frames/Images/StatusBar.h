#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

wxDECLARE_EVENT(wxEVT_TOGGLE_TRIANGULATE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_LOCK, wxCommandEvent);

class Configuration;
class StatusBar final: public wxFrame
{
  public:
	StatusBar(wxWindow* parent, const wxPoint& position, std::shared_ptr<Configuration> theConfiguration);

	void setSourceZoom(int zoomLevel) const;
	void setTargetZoom(int zoomLevel) const;
	void setTriangulationSane(bool sane);

	[[nodiscard]] auto isTriangulate() const { return triangulate; }

  private:
	void onMove(wxMoveEvent& event);
	void onZoomChanged(const wxCommandEvent& evt);
	void onZoomResetButton(const wxCommandEvent& evt);
	void onTriangulate(wxCommandEvent& evt);
	void onClose(wxCloseEvent& event);
	void onLock(wxCommandEvent& evt);

	bool triangulate = false;
	bool lock = false;

	wxTextCtrl* sourceZoomField = nullptr;
	wxTextCtrl* targetZoomField = nullptr;
	wxButton* triangulateButton = nullptr;

	wxButton* lockButton = nullptr;
	wxWindow* lockStatus = nullptr;

	wxStaticText* triangulateText = nullptr;

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};
