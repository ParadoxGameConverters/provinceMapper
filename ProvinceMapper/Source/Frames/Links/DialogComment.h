#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

wxDECLARE_EVENT(wxEVT_UPDATE_COMMENT, wxCommandEvent);

class DialogComment: public wxDialog
{
  public:
	DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row);

  private:
	void onCancel(wxCommandEvent& evt);
	void onSave(wxCommandEvent& evt);

	wxTextCtrl* tc = nullptr;
	int ID = 0;
	
  protected:
	wxEvtHandler* eventHandler = nullptr;
};