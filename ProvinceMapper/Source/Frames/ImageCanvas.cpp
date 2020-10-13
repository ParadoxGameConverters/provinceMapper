#include "ImageCanvas.h"
#include "../LinkMapper/LinkMappingVersion.h"

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
	imageData = new unsigned char[width * height * 3];
	memcpy(imageData, image->GetData(), width * height * 3);
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
