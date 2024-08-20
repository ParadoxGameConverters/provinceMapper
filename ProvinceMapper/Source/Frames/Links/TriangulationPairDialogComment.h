#pragma once
#include "DialogCommentBase.h"

wxDECLARE_EVENT(wxEVT_UPDATE_TRIANGULATION_PAIR_COMMENT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR_COMMENT, wxCommandEvent);

class TriangulationPairDialogComment final: public DialogCommentBase
{
  public:
	TriangulationPairDialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, int row);
	TriangulationPairDialogComment(wxWindow* parent, const wxString& title, int row);

  protected:
	wxEventTypeTag<wxCommandEvent> getUpdateNameCommand() override { return wxEVT_UPDATE_TRIANGULATION_PAIR_COMMENT; }
	wxEventTypeTag<wxCommandEvent> getAddCommentCommand() override { return wxEVT_ADD_TRIANGULATION_PAIR_COMMENT; }
};
