#include "ImageCanvas.h"
#include "Definitions/Definitions.h"
#include "Frames/Links/LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_SELECT_PROVINCE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DESELECT_PROVINCE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_ID, wxCommandEvent);

ImageCanvas::ImageCanvas(wxWindow* parent,
	 ImageTabSelector theSelector,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* theImage,
	 std::shared_ptr<Definitions> theDefinitions):
	 wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL),
	 definitions(std::move(theDefinitions)), eventListener(parent)
{
	Bind(wxEVT_MOTION, &ImageCanvas::onMouseOver, this);
	Bind(wxEVT_LEFT_UP, &ImageCanvas::leftUp, this);

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
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
		imageData[offset] = 0;
		imageData[offset + 1] = 0;
		imageData[offset + 2] = 0;
	}
}

void ImageCanvas::restoreImageData()
{
	memcpy(imageData, image->GetData(), imageDataSize);
}

void ImageCanvas::activateLinkByIndex(const int row)
{
	if (activeVersion && row < static_cast<int>(activeVersion->getLinks()->size()))
	{
		activeLink = activeVersion->getLinks()->at(row);
		// Strafe our provinces' pixels.
		strafeProvinces();
	}
}

void ImageCanvas::activateLinkByID(const int ID)
{
	if (!activeVersion)
		return;
	for (const auto& link: *activeVersion->getLinks())
	{
		if (link->getID() == ID)
		{
			activeLink = link;
			// Strafe our provinces' pixels.
			strafeProvinces();
			break;
		}
	}
}

void ImageCanvas::strafeProvinces()
{
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

void ImageCanvas::applyStrafedPixels()
{
	for (const auto& pixel: strafedPixels)
	{
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
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
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
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

void ImageCanvas::onMouseOver(wxMouseEvent& event)
{
	const auto x = CalcUnscrolledPosition(event.GetPosition()).x;
	const auto y = CalcUnscrolledPosition(event.GetPosition()).y;
	const auto offs = coordsToOffset(x, y, width);
	// We may be out of scope if mouse leaves canvas.
	if (x >= 0 && x <= width - 1 && y >= 0 && y <= height - 1)
	{
		const auto chroma = pixelPack(image->GetData()[offs], image->GetData()[offs + 1], image->GetData()[offs + 2]);

		// cache?
		std::string name;
		if (tooltipCache.first == chroma)
			name = tooltipCache.second;
		else
		{
			// poke the definitions for a chroma name.
			const auto& provinceName = definitions->getNameForChroma(chroma);
			if (provinceName)
				name = *provinceName;
			else
				name = "UNDEFINED";
			tooltipCache = std::pair(chroma, name);
		}

		this->SetToolTip(name);
	}
	event.Skip();
}

void ImageCanvas::leftUp(wxMouseEvent& event)
{
	// Left up means:
	// 1. select a mapping, or
	// 2. add a province to the existing mapping, or
	// 3. remove it from active mapping.

	// What province have we clicked?
	const auto x = CalcUnscrolledPosition(event.GetPosition()).x;
	const auto y = CalcUnscrolledPosition(event.GetPosition()).y;
	const auto offs = coordsToOffset(x, y, width);
	// We may be out of scope if mouse leaves canvas.
	if (x >= 0 && x <= width - 1 && y >= 0 && y <= height - 1)
	{
		const auto chroma = pixelPack(image->GetData()[offs], image->GetData()[offs + 1], image->GetData()[offs + 2]);
		// Province may not even be defined/instanced, let alone linked. Tread carefully.
		const auto province = definitions->getProvinceForChroma(chroma);
		if (!province || !activeVersion) // Do we have an active version? If not, we're in boo-boo. Same for undefined province.
		{
			// Play dead.
			event.Skip();
			return;
		}

		// Case 1: DESELECT PROVINCE if we have an active link with this province inside.
		if (activeLink)
		{
			std::vector<std::shared_ptr<Province>> interestingProvinces;
			if (selector == ImageTabSelector::SOURCE)
				interestingProvinces = activeLink->getSources();
			else
				interestingProvinces = activeLink->getTargets();
			for (const auto& interestingProvince: interestingProvinces)
				if (*interestingProvince == *province)
				{
					// Trigger deselect procedure.
					deselectProvince(province);
					return;
				}
		}

		// Case 2: SELECT LINK if province is linked elsewhere.
		for (const auto& link: *activeVersion->getLinks())
		{
			std::vector<std::shared_ptr<Province>> interestingProvinces;
			if (selector == ImageTabSelector::SOURCE)
				interestingProvinces = link->getSources();
			else
				interestingProvinces = link->getTargets();
			for (const auto& interestingProvince: interestingProvinces)
			{
				if (*interestingProvince == *province)
				{
					// trigger select link procedure.
					selectLink(link->getID());
					return;
				}
			}
		}

		// Case 3: SELECT PROVINCE since it's not linked anywhere.
		selectProvince(province);
	}
}

void ImageCanvas::selectLink(int linkID)
{
	auto* evt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_ID);
	evt->SetInt(linkID);
	eventListener->QueueEvent(evt->Clone());
}

void ImageCanvas::selectProvince(std::shared_ptr<Province> province)
{
	Log(LogLevel::Debug) << "Select Province";
}

void ImageCanvas::deselectProvince(std::shared_ptr<Province> province)
{
	Log(LogLevel::Debug) << "Deselect Province";
}
