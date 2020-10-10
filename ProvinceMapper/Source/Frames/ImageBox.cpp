#include "ImageBox.h"
#include "Log.h"
#include "wx/image.h"

ImageBox::ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector): wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	// ImageBox is the raw image + display functions.
	
	eventListener = parent; // command events will propagate up the thread to someone who's listening, likely mainframe.
	selector = theSelector; // We should know which of the images we are.

	Bind(wxEVT_PAINT, &ImageBox::paintEvent, this);
	Bind(wxEVT_SIZE, &ImageBox::onSize, this);

	image = theImage;
	dc = new wxClientDC(this); // Device Context, a drawable area.
}

void ImageBox::paintEvent(wxPaintEvent& evt)
{
	render();
}
void ImageBox::paintNow()
{
	render();
}
void ImageBox::render()
{
	dc->DrawBitmap(image, 0, 0, false);
	SetMinSize(image.GetSize());
}
void ImageBox::onSize(wxSizeEvent& evt)
{
	paintNow();
	evt.Skip();
}

