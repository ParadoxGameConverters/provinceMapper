#include "ImageCanvas.h"
#include "../../LinkMapper/LinkMappingVersion.h"

ImageCanvas::ImageCanvas(wxWindow* parent,
	 ImageTabSelector theSelector,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* theImage):
	 wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL)
{
	// ImageBox is the raw image + display functions.
	image = theImage;
	width = image->GetSize().GetX();
	height = image->GetSize().GetY();
	imageDataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(3);
	imageData = new unsigned char[imageDataSize];
	memcpy(imageData, image->GetData(), imageDataSize);
	activeVersion = theActiveVersion;
	selector = theSelector; // We should know which of the images we are.
}

void ImageCanvas::generateBlackList()
{
	blackList.clear();
	for (const auto& link: *activeVersion->getLinks())
	{
		std::vector<std::shared_ptr<Province>> pixelSources;
		if (selector == ImageTabSelector::SOURCE)
			pixelSources = link->getSources();
		else if (selector == ImageTabSelector::TARGET)
			pixelSources = link->getTargets();
		for (const auto& province: pixelSources)
			for (const auto& pixel: province->pixels)
				blackList.emplace_back(pixel);
	}
}

void ImageCanvas::applyBlackList()
{
	for (const auto& pixel: blackList)
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