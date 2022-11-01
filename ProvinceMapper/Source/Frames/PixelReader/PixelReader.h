#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Definitions/DefinitionsInterface.h"
#include "Frames/Images/ImageCanvas.h"
#include <wx/thread.h>

class PixelReader final: public wxThread
{
  public:
	PixelReader(wxEvtHandler* parent): wxThread(wxTHREAD_JOINABLE), eventHandler(parent) {}
	void prepare(wxImage* theImage, const std::shared_ptr<DefinitionsInterface>& theDefinitions)
	{
		image = theImage;
		definitions = theDefinitions;
	}

  private:
	void* Entry() override;
	wxImage* image = nullptr;
	std::shared_ptr<DefinitionsInterface> definitions;

	[[nodiscard]] bool isSameColorAtCoords(int ax, int ay, int bx, int by) const;


  protected:
	wxEvtHandler* eventHandler;
};
