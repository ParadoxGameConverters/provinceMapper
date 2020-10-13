#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "ImageBox.h"
#include "../LinkMapper/LinkMappingVersion.h"

class ImageTab: public wxScrolledWindow
{
  public:
	ImageTab(wxWindow* parent, ImageTabSelector selector, const std::shared_ptr<LinkMappingVersion>& theActiveVersion);
	//void refresh();

  private:
	ImageBox* imageBox = nullptr;
};
