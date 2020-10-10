#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

enum class ImageTabSelector
{
	SOURCE,
	TARGET
};

class ImageBox: public wxWindow
{
  public:
	ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector);
	void paintNow();

  private:
	void paintEvent(wxPaintEvent& evt);
	void onSize(wxSizeEvent& event);
	void render();

	wxClientDC* dc = nullptr;
	wxImage image;
	ImageTabSelector selector;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
