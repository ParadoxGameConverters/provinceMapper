#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "DialogCommentBase.h"

wxDECLARE_EVENT(wxEVT_UPDATE_NAME, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_COMMENT, wxCommandEvent);

class DialogComment final : public DialogCommentBase
{
  public:
	DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row);
	DialogComment(wxWindow* parent, const wxString& title, const int row);

  protected:
	wxEventTypeTag<wxCommandEvent> getUpdateNameCommand() { return wxEVT_UPDATE_NAME; }
	wxEventTypeTag<wxCommandEvent> getAddCommentCommand() { return wxEVT_ADD_COMMENT; }
};