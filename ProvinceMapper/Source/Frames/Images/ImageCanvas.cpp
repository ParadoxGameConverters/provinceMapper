#include "ImageCanvas.h"
#include "LinkMapper/LinkMappingVersion.h"

ImageCanvas::ImageCanvas(wxWindow* parent,
	 ImageTabSelector theSelector,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* theImage):
	 wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL)
{
	image = theImage;
	width = image->GetSize().GetX();
	height = image->GetSize().GetY();
	imageDataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(3);
	imageData = new unsigned char[imageDataSize];
	memcpy(imageData, image->GetData(), imageDataSize);
	activeVersion = theActiveVersion;
	selector = theSelector; // We should know which of the images we are.
}

void ImageCanvas::generateShadedPixels()
{
	shadedPixels.clear();
	for (const auto& link: *activeVersion->getLinks())
	{
		std::vector<std::shared_ptr<Province>> pixelSources;
		if (selector == ImageTabSelector::SOURCE)
			pixelSources = link->getSources();
		else if (selector == ImageTabSelector::TARGET)
			pixelSources = link->getTargets();
		for (const auto& province: pixelSources)
			for (const auto& pixel: province->innerPixels)
				shadedPixels.emplace_back(pixel);
	}
}

void ImageCanvas::applyShadedPixels()
{
	for (const auto& pixel: shadedPixels)
	{
		const auto offset = (pixel.x + width * pixel.y) * 3;
		imageData[offset] = 0;
		imageData[offset + 1] = 0;
		imageData[offset + 2] = 0;
	}
}

void ImageCanvas::restoreImageData()
{
	memcpy(imageData, image->GetData(), imageDataSize);
}

void ImageCanvas::activateLink(int row)
{
	if (activeVersion && row < static_cast<int>(activeVersion->getLinks()->size()))
	{
		activeLink = activeVersion->getLinks()->at(row);
		// Strafe our provinces' pixels.
		std::vector<std::shared_ptr<Province>> pixelSources;
		if (selector == ImageTabSelector::SOURCE)
			pixelSources = activeLink->getSources();
		else if (selector == ImageTabSelector::TARGET)
			pixelSources = activeLink->getTargets();
		for (const auto& province: pixelSources)
			for (const auto& pixel: province->innerPixels)
				if ((pixel.x + pixel.y) % 8 == 0)
					strafedPixels.emplace_back(pixel);
	}
}

void ImageCanvas::applyStrafedPixels()
{
	for (const auto& pixel: strafedPixels)
	{
		const auto offset = (pixel.x + width * pixel.y) * 3;
		imageData[offset] = 255;
		imageData[offset + 1] = 255;
		imageData[offset + 2] = 255;
	}
}

void ImageCanvas::deactivateLink()
{
	activeLink.reset();
	// restore color
	for (const auto& pixel: strafedPixels)
	{
		const auto offset = (pixel.x + width * pixel.y) * 3;
		if (black)
		{
			imageData[offset] = 0;
			imageData[offset + 1] = 0;
			imageData[offset + 2] = 0;			
		}
		else
		{
			imageData[offset] = pixel.r;
			imageData[offset + 1] = pixel.g;
			imageData[offset + 2] = pixel.b;			
		}
	}	
	strafedPixels.clear();
}
