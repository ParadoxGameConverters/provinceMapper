#include "ImageFrame.h"
#include "OSCompatibilityLayer.h"
#include <wx/dcbuffer.h>
#include <wx/splitter.h>

MyFrame::MyFrame(wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style): wxFrame(parent, id, title, pos, size, style)
{
	m_bitmap = wxBitmap("test-from/provinces.bmp", wxBITMAP_TYPE_BMP);
	m_bitmap2 = wxBitmap("test-to/provinces.bmp", wxBITMAP_TYPE_BMP);

	wxSplitterWindow* m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

	m_canvas = new wxScrolledCanvas(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL);
	m_canvas2 = new wxScrolledCanvas(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL);

	m_canvas->SetScrollRate(5, 5);
	m_canvas->SetVirtualSize(m_bitmap.GetWidth(), m_bitmap.GetHeight());
	m_canvas->SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_canvas->Bind(wxEVT_PAINT, &MyFrame::OnScrollPaint, this);
	m_canvas2->SetScrollRate(5, 5);
	m_canvas2->SetVirtualSize(m_bitmap2.GetWidth(), m_bitmap2.GetHeight());
	m_canvas2->SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_canvas2->Bind(wxEVT_PAINT, &MyFrame::OnScrollPaint, this);

	m_splitter->SplitVertically(m_canvas, m_canvas2, GetSize().x / 2);
}

void MyFrame::OnScrollPaint(wxPaintEvent& event)
{
	wxAutoBufferedPaintDC dc(m_canvas);
	m_canvas->DoPrepareDC(dc);
	dc.Clear();
	dc.DrawBitmap(m_bitmap, 0, 0);
	wxAutoBufferedPaintDC dc2(m_canvas2);
	m_canvas2->DoPrepareDC(dc2);
	dc2.Clear();
	dc2.DrawBitmap(m_bitmap2, 0, 0);
}