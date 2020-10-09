#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "ImageBox.h"

class ImageTab: public wxScrolledWindow
{
  public:
	ImageTab(wxWindow* parent, ImageTabSelector selector);
	void registerPoint(const Point& point);
	void refresh();
	void updatePoint(wxCommandEvent& event);

  private:
	ImageBox* imageBox = nullptr;
};
