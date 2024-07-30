#include "TriangulationPairDialogComment.h"

wxDEFINE_EVENT(wxEVT_UPDATE_TRIANGULATION_PAIR_COMMENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_TRIANGULATION_PAIR_COMMENT, wxCommandEvent);

TriangulationPairDialogComment::TriangulationPairDialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row):
	 DialogCommentBase(parent, title, theComment, row)
{
}


TriangulationPairDialogComment::TriangulationPairDialogComment(wxWindow* parent, const wxString& title, const int row): DialogCommentBase(parent, title, row)
{
}