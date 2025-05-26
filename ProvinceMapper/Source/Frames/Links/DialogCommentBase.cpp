#include "DialogCommentBase.h"
#include <Log.h>

DialogCommentBase::DialogCommentBase(wxWindow* parent, const wxString& title, const std::string& theComment, const int row):
	 wxDialog(parent, -1, title, wxDefaultPosition, wxSize(250, 170)), ID(row), eventHandler(parent)
{
	initFrame();
	tc->SetValue(theComment);
	okButton->Bind(wxEVT_BUTTON, &DialogCommentBase::onUpdate, this);
}

DialogCommentBase::DialogCommentBase(wxWindow* parent, const wxString& title, const int row):
	 wxDialog(parent, -1, title, wxDefaultPosition, wxSize(250, 170)), ID(row), eventHandler(parent)
{
	initFrame();
	updateMode = false;
	okButton->Bind(wxEVT_BUTTON, &DialogCommentBase::onAdd, this);
}

void DialogCommentBase::initFrame()
{
	auto* panel = new wxPanel(this, -1);

	auto* vbox = new wxBoxSizer(wxVERTICAL);
	auto* hbox = new wxBoxSizer(wxHORIZONTAL);

	auto* st = new wxStaticBox(panel, -1, "Edit Text:", wxPoint(5, 5), wxSize(220, 50));
	tc = new wxTextCtrl(st, -1, wxEmptyString, wxPoint(20, 20), wxSize(180, 20), wxTE_PROCESS_ENTER);
	tc->Bind(wxEVT_KEY_DOWN, &DialogCommentBase::onKeyDown, this);
	tc->Bind(wxEVT_COMMAND_TEXT_ENTER, &DialogCommentBase::onEnter, this);

	okButton = new wxButton(this, -1, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
	auto* closeButton = new wxButton(this, -1, wxT("Can Sale"), wxDefaultPosition, wxSize(70, 30));
	closeButton->Bind(wxEVT_BUTTON, &DialogCommentBase::onCancel, this);

	hbox->Add(okButton, 1);
	hbox->Add(closeButton, 1, wxLEFT, 5);

	vbox->Add(panel, 1);
	vbox->Add(hbox, wxSizerFlags(0).CenterHorizontal().Border(wxTOP | wxBOTTOM, 10));

	SetSizer(vbox);

	Centre();
}

void DialogCommentBase::onCancel(wxCommandEvent& evt)
{
	Destroy();
}

void DialogCommentBase::onUpdate(wxCommandEvent& evt)
{
	executeUpdate();
}

void DialogCommentBase::onAdd(wxCommandEvent& evt)
{
	executeAdd();
}

void DialogCommentBase::executeUpdate()
{
	auto* event = new wxCommandEvent(getUpdateNameCommand());
	event->SetString(tc->GetValue());
	event->SetInt(ID);
	eventHandler->QueueEvent(event->Clone());
	Destroy();
}

void DialogCommentBase::executeAdd()
{
	auto* event = new wxCommandEvent(getAddCommentCommand());
	event->SetString(tc->GetValue());
	event->SetInt(ID);
	eventHandler->QueueEvent(event->Clone());
	Destroy();
}

void DialogCommentBase::onKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_ESCAPE:
			Destroy();
			break;
		default:
			event.Skip();
	}
}

void DialogCommentBase::onEnter(wxCommandEvent& evt)
{
	if (updateMode)
		executeUpdate();
	else
		executeAdd();
}
