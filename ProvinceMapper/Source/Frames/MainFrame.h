#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "Definitions/Definitions.h"
#include "LinkMapper/LinkMapper.h"
#include <wx/splitter.h>

enum class ImageTabSelector;
class ImageFrame;
class LinksFrame;
class MainFrame: public wxFrame
{
  public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	void initFrame();

  private:
	void onExit(wxCommandEvent& event);
	void onAbout(wxCommandEvent& event);
	void onSupportUs(wxCommandEvent& event);
	void initImageFrame(wxCommandEvent& event);
	void initLinksFrame(wxCommandEvent& event);

	void readPixels(ImageTabSelector selector, const wxImage& img);
	static int coordsToOffset(int x, int y, int width);
	static bool isSameColorAtCoords(int ax, int ay, int bx, int by, const wxImage& img);

	LinksFrame* linksFrame = nullptr;
	wxFlexGridSizer* vbox = nullptr;
	
	ImageFrame* imageFrame = nullptr;
	wxImage* sourceImg = nullptr;
	wxImage* targetImg = nullptr;

	Definitions sourceDefs;
	Definitions targetDefs;
	LinkMapper linkMapper;
};
