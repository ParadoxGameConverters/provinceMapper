#pragma once
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class LinkMappingVersion;
class wxSplitterWindow;
class ImageCanvas;
enum class ImageTabSelector;

class ImageFrame: public wxFrame
{
  public:
	ImageFrame(wxWindow* parent, const std::shared_ptr<LinkMappingVersion>& theActiveVersion, wxImage* sourceImg, wxImage* targetImg);

	void render();

  private:
	void onScrollPaint(wxPaintEvent& event);
	void onToggleOrientation(wxCommandEvent& event);
	void onToggleBlack(wxCommandEvent& event);
	void onClose(wxCloseEvent& event);

	ImageCanvas* sourceCanvas;
	ImageCanvas* targetCanvas;
	wxSplitterWindow* splitter;

	bool black = false;

  protected:
	wxEvtHandler* eventHandler;
};
