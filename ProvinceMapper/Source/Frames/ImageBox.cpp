#include "ImageBox.h"
#include "Log.h"
#include "wx/image.h"

ImageBox::ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector, const std::shared_ptr<LinkMappingVersion>& theActiveVersion):
	 wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	// ImageBox is the raw image + display functions.

	activeVersion = theActiveVersion;
	eventListener = parent; // command events will propagate up the thread to someone who's listening, likely mainframe.
	selector = theSelector; // We should know which of the images we are.
	Bind(wxEVT_PAINT, &ImageBox::paintEvent, this);
	//Bind(wxEVT_SIZE, &ImageBox::onSize, this);

	auto dataSize = theImage.GetSize().GetX() * theImage.GetSize().GetY() * 3;
	imageData = new unsigned char[dataSize];
	memcpy(imageData, theImage.GetData(), dataSize);	
	width = theImage.GetSize().GetX();
	height = theImage.GetSize().GetY();
	dc = new wxClientDC(this); // Device Context, a drawable area.

	generateBlackList();
}

void ImageBox::paintEvent(wxPaintEvent& evt)
{
	render();
}
void ImageBox::paintNow() const
{
	render();
}
void ImageBox::render() const
{
	const wxImage bmp(width, height, imageData, true);
	dc->DrawBitmap(bmp, 0, 0, false);
}
void ImageBox::onSize(wxSizeEvent& evt)
{
	evt.Skip();
	//paintNow();
}

void ImageBox::generateBlackList()
{
	blackList.clear();
	for (const auto& link: *activeVersion->getLinks())
	{
		std::vector<std::shared_ptr<Province>> pixelSources;
		if (selector == ImageTabSelector::SOURCE)
		{
			pixelSources = link->getSources();
		}
		else if (selector == ImageTabSelector::TARGET)
		{
			pixelSources = link->getTargets();
		}
		for (const auto& province: pixelSources)
		{
			for (const auto& pixel: province->pixels)
			{
				const auto offs = 3 * (pixel.y * width + pixel.x);
				imageData[offs] = 0;
				imageData[offs + 1] = 0;
				imageData[offs + 2] = 0;
			}
		}
	}
}
