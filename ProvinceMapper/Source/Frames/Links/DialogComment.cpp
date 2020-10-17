#include "DialogComment.h"

wxDEFINE_EVENT(wxEVT_UPDATE_COMMENT, wxCommandEvent);

DialogComment::DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row):
	 wxDialog(parent, -1, title, wxDefaultPosition, wxSize(250, 170)), ID(row), eventHandler(parent)
{
	auto* panel = new wxPanel(this, -1);

	auto* vbox = new wxBoxSizer(wxVERTICAL);
	auto* hbox = new wxBoxSizer(wxHORIZONTAL);

	auto* st = new wxStaticBox(panel, -1, "Comment", wxPoint(5, 5), wxSize(220, 50));
	tc = new wxTextCtrl(st, -1, theComment, wxPoint(20, 20), wxSize(180, 20));

	auto* okButton = new wxButton(this, -1, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
	okButton->Bind(wxEVT_BUTTON, &DialogComment::onSave, this);
	auto* closeButton = new wxButton(this, -1, wxT("Can Sale"), wxDefaultPosition, wxSize(70, 30));
	closeButton->Bind(wxEVT_BUTTON, &DialogComment::onCancel, this);
	
	hbox->Add(okButton, 1);
	hbox->Add(closeButton, 1, wxLEFT, 5);

	vbox->Add(panel, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);

	Centre();
}

void DialogComment::onCancel(wxCommandEvent& evt)
{
	Destroy();
}

void DialogComment::onSave(wxCommandEvent& evt)
{
	auto* event = new wxCommandEvent(wxEVT_UPDATE_COMMENT);
	event->SetString(tc->GetValue());
	event->SetInt(ID);
	eventHandler->QueueEvent(event->Clone());
	Destroy();
}
