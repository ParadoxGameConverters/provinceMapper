#pragma once
#include <wx/wxprec.h>

#include "../LinkMapper/LinkMappingVersion.h"
#include "../Provinces/Province.h"
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
	ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector, const std::shared_ptr<LinkMappingVersion>& theActiveVersion);
	void paintNow();

  private:
	void paintEvent(wxPaintEvent& evt);
	void onSize(wxSizeEvent& event);
	void render();
	void generateBlackList();

	wxClientDC* dc = nullptr;
	unsigned char* imageData;
	int height = 0;
	int width = 0;
	ImageTabSelector selector;
	std::shared_ptr<LinkMappingVersion> activeVersion;
	std::vector<Pixel> blackList;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
