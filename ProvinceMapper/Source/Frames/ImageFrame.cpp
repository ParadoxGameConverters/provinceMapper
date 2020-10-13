#include "ImageFrame.h"
#include "OSCompatibilityLayer.h"
#include <wx/dcbuffer.h>
#include <wx/splitter.h>

#include "ImageCanvas.h"

ImageFrame::ImageFrame(wxWindow* parent, const std::shared_ptr<LinkMappingVersion>& theActiveVersion, wxImage* sourceImg, wxImage* targetImg):
	 wxFrame(parent, wxID_ANY, "Provinces", wxDefaultPosition, wxSize(1200, 800), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
	auto* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

	sourceCanvas = new ImageCanvas(splitter, ImageTabSelector::SOURCE, theActiveVersion, sourceImg);
	targetCanvas = new ImageCanvas(splitter, ImageTabSelector::TARGET, theActiveVersion, targetImg);

	sourceCanvas->SetScrollRate(50, 50);
	sourceCanvas->SetVirtualSize(sourceCanvas->getWidth(), sourceCanvas->getHeight());
	sourceCanvas->SetBackgroundStyle(wxBG_STYLE_PAINT);
	sourceCanvas->Bind(wxEVT_PAINT, &ImageFrame::OnScrollPaint, this);
	targetCanvas->SetScrollRate(50, 50);
	targetCanvas->SetVirtualSize(targetCanvas->getWidth(), targetCanvas->getHeight());
	targetCanvas->SetBackgroundStyle(wxBG_STYLE_PAINT);
	targetCanvas->Bind(wxEVT_PAINT, &ImageFrame::OnScrollPaint, this);

	splitter->SetMinSize(wxSize(1200, 800));
	splitter->SplitVertically(sourceCanvas, targetCanvas, GetSize().x / 2);
}

void ImageFrame::OnScrollPaint(wxPaintEvent& event)
{
	render();
}

void ImageFrame::render()
{
	wxAutoBufferedPaintDC sourceDC(sourceCanvas);
	sourceCanvas->DoPrepareDC(sourceDC);
	sourceDC.Clear();
	const wxImage bmp(sourceCanvas->getWidth(), sourceCanvas->getHeight(), sourceCanvas->getImageData(), true);
	sourceDC.DrawBitmap(bmp, 0, 0);

	wxAutoBufferedPaintDC targetDC(targetCanvas);
	targetCanvas->DoPrepareDC(targetDC);
	targetDC.Clear();
	const wxImage bmp2(targetCanvas->getWidth(), targetCanvas->getHeight(), targetCanvas->getImageData(), true);
	targetDC.DrawBitmap(bmp2, 0, 0);
}
