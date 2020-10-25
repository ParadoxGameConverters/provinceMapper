#include "ProvinceMapper.h"

#include "Frames/MainFrame.h"
#include "Log.h"

wxIMPLEMENT_APP(ProvinceMapper);

bool ProvinceMapper::OnInit()
{
	try
	{
		wxInitAllImageHandlers();
		auto* frame = new MainFrame("ProvinceMapper", wxPoint(50, 50), wxSize(630, 250));
		frame->initFrame();
		frame->SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));

		auto* menuFile = new wxMenu;
		menuFile->Append(wxID_EXIT, "Exit");
		auto* menuHelp = new wxMenu;
		menuHelp->Append(wxID_ABOUT, "About");
		menuHelp->Append(wxID_NETWORK, "Support Us");
		auto* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, "&Menu");
		menuBar->Append(menuHelp, "&PGCG");

		frame->SetMenuBar(menuBar);
		frame->CreateStatusBar();
		frame->SetStatusText("Paradox Game Converters Group");

		frame->Show(true);
		return true;
	}
	catch (std::exception& e)
	{
		Log(LogLevel::Error) << "Exception during execution: " << e.what();
		return false;
	}
}
