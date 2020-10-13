#pragma once
#include <wx/wxprec.h>

#include "../LinkMapper/LinkMappingVersion.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


class ImageCanvas;
enum class ImageTabSelector;

class ImageFrame: public wxFrame
{
  public:
	ImageFrame(wxWindow* parent, const std::shared_ptr<LinkMappingVersion>& theActiveVersion, wxImage* sourceImg, wxImage* targetImg);

	void render();
	
  private:
	void OnScrollPaint(wxPaintEvent& event);

	ImageCanvas* sourceCanvas;
	ImageCanvas* targetCanvas;
};
