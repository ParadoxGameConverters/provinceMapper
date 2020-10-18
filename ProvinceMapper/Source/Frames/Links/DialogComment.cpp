#include "DialogComment.h"
#include "Log.h"

wxDEFINE_EVENT(wxEVT_UPDATE_COMMENT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_ADD_COMMENT, wxCommandEvent);

DialogComment::DialogComment(wxWindow* parent, const wxString& title, const std::string& theComment, const int row):
	 wxDialog(parent, -1, title, wxDefaultPosition, wxSize(250, 170)), ID(row), eventHandler(parent)
{
	initFrame();
	tc->SetValue(theComment);
	okButton->Bind(wxEVT_BUTTON, &DialogComment::onUpdate, this);
}

DialogComment::DialogComment(wxWindow* parent, const wxString& title, const int row):
	 wxDialog(parent, -1, title, wxDefaultPosition, wxSize(250, 170)), ID(row), eventHandler(parent)
{
	initFrame();
	updateMode = false;
	okButton->Bind(wxEVT_BUTTON, &DialogComment::onAdd, this);
}

void DialogComment::initFrame()
{
	auto* panel = new wxPanel(this, -1);

	auto* vbox = new wxBoxSizer(wxVERTICAL);
	auto* hbox = new wxBoxSizer(wxHORIZONTAL);

	auto* st = new wxStaticBox(panel, -1, "Comment", wxPoint(5, 5), wxSize(220, 50));
	tc = new wxTextCtrl(st, -1, wxEmptyString, wxPoint(20, 20), wxSize(180, 20), wxTE_PROCESS_ENTER);
	tc->Bind(wxEVT_KEY_DOWN, &DialogComment::onKeyDown, this);
	tc->Bind(wxEVT_COMMAND_TEXT_ENTER, &DialogComment::onEnter, this);

	okButton = new wxButton(this, -1, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
	auto* closeButton = new wxButton(this, -1, wxT("Can Sale"), wxDefaultPosition, wxSize(70, 30));
	closeButton->Bind(wxEVT_BUTTON, &DialogComment::onCancel, this);

	hbox->Add(okButton, 1);
	hbox->Add(closeButton, 1, wxLEFT, 5);

	vbox->Add(panel, 1);
	vbox->Add(hbox, wxSizerFlags(0).CenterHorizontal().Border(wxTOP | wxBOTTOM, 10));

	SetSizer(vbox);

	Centre();
}

void DialogComment::onCancel(wxCommandEvent& evt)
{
	Destroy();
}

void DialogComment::onUpdate(wxCommandEvent& evt)
{
	executeUpdate();
}

void DialogComment::onAdd(wxCommandEvent& evt)
{
	executeAdd();
}

void DialogComment::executeUpdate()
{
	auto* event = new wxCommandEvent(wxEVT_UPDATE_COMMENT);
	event->SetString(tc->GetValue());
	event->SetInt(ID);
	eventHandler->QueueEvent(event->Clone());
	Destroy();
}

void DialogComment::executeAdd()
{
	auto* event = new wxCommandEvent(wxEVT_ADD_COMMENT);
	event->SetString(tc->GetValue());
	event->SetInt(ID);
	eventHandler->QueueEvent(event->Clone());
	Destroy();
}

void DialogComment::onKeyDown(wxKeyEvent& event)
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

void DialogComment::onEnter(wxCommandEvent& evt)
{
	if (updateMode)
		executeUpdate();
	else
		executeAdd();
}
