#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class DialogCommentBase : public wxDialog
{
  public:
	DialogCommentBase(wxWindow* parent, const wxString& title, const std::string& theComment, const int row);
	DialogCommentBase(wxWindow* parent, const wxString& title, const int row);

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

	virtual wxEventTypeTag<wxCommandEvent> getUpdateNameCommand();
	virtual wxEventTypeTag<wxCommandEvent> getAddCommentCommand();
};