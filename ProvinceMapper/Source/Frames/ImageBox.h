#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "PointData.h"

wxDECLARE_EVENT(wxEVT_POINT_PLACED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DESELECT_POINT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DELETE_WORKING_POINT, wxCommandEvent);

class ImageBox: public wxWindow
{
  public:
	ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector);
	void registerPoint(const Point& point);
	void paintNow();
	void updatePoint(wxCommandEvent& event);

  private:
	void paintEvent(wxPaintEvent& evt);
	void onSize(wxSizeEvent& event);
	void render();
	void rightUp(wxMouseEvent& event);
	void leftUp(wxMouseEvent& event);

	wxClientDC* dc = nullptr;
	wxImage image;
	ImageTabSelector selector;
	std::vector<Point> knownPoints;
	void onKeyDown(wxKeyEvent& event);

  protected:
	wxEvtHandler* eventListener = nullptr;
};
