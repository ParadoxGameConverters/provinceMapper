#include "DialogComment.h"
#include "Log.h"

wxDEFINE_EVENT(wxEVT_UPDATE_NAME, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_COMMENT, wxCommandEvent);

DialogComment::DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row): 
	DialogCommentBase(parent, title, theComment, row)
{
}


DialogComment::DialogComment(wxWindow* parent, const wxString& title, const int row):
	DialogCommentBase(parent, title, row)
{
}