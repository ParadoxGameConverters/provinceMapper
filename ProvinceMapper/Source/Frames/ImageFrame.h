#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


class MyFrame: public wxFrame
{
  public:
	MyFrame(wxWindow* parent,
		 int id = wxID_ANY,
		 wxString title = "Provinces",
		 wxPoint pos = wxDefaultPosition,
		 wxSize size = wxDefaultSize,
		 int style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

  private:
	void OnScrollPaint(wxPaintEvent& event);

	wxScrolledCanvas* m_canvas;
	wxScrolledCanvas* m_canvas2;
	wxBitmap m_bitmap;
	wxBitmap m_bitmap2;
};