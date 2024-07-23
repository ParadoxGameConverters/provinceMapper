#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "DialogCommentBase.h";

wxDECLARE_EVENT(wxEVT_UPDATE_NAME, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_ADD_COMMENT, wxCommandEvent);

class DialogComment final : public DialogCommentBase
{
  public:
	DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row);
	DialogComment(wxWindow* parent, const wxString& title, const int row);

  private:
	void onCancel(wxCommandEvent& evt);
	void onUpdate(wxCommandEvent& evt);
	void onAdd(wxCommandEvent& evt);
	void onKeyDown(wxKeyEvent& event);
	void onEnter(wxCommandEvent& evt);

	void initFrame();
	void executeUpdate();
	void executeAdd();

	wxTextCtrl* tc = nullptr;
	wxButton* okButton = nullptr;
	int ID = 0;
	bool updateMode = true;

  protected:
	wxEvtHandler* eventHandler = nullptr;

	wxEventTypeTag<wxCommandEvent> getUpdateNameCommand() { return wxEVT_UPDATE_NAME; }
	wxEventTypeTag<wxCommandEvent> getAddCommentCommand() { return wxEVT_ADD_COMMENT; }
};