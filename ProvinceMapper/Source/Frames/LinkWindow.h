#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../LinkMapper/LinkMappingVersion.h"
#include "ImageBox.h"
#include <wx/grid.h>

wxDECLARE_EVENT(wxEVT_CHANGE_TAB, wxCommandEvent);

class LinkWindow: public wxWindow
{
  public:
	LinkWindow(wxWindow* parent, const std::shared_ptr<LinkMappingVersion>& theActiveVersion);
	void redrawGrid() const;

  private:
	wxGrid* theGrid = nullptr;
	std::shared_ptr<LinkMappingVersion> activeVersion;

  protected:
	wxEvtHandler* eventListener = nullptr;
};
