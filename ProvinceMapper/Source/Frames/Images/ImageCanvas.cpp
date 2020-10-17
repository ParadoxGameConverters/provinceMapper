#include "ImageCanvas.h"
#include "Definitions/Definitions.h"
#include "Frames/Links/LinksTab.h"
#include "LinkMapper/LinkMappingVersion.h"
#include "Log.h"
#include "Provinces/Province.h"

wxDEFINE_EVENT(wxEVT_TOGGLE_PROVINCE, wxCommandEvent);
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
	Bind(wxEVT_RIGHT_UP, &ImageCanvas::rightUp, this);

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
		for (const auto& province: getRelevantProvinces(link))
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

void ImageCanvas::restoreImageData() const
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
	for (const auto& province: getRelevantProvinces(activeLink))
		strafeProvince(province);
}

void ImageCanvas::strafeProvince(const std::shared_ptr<Province>& province)
{
	for (const auto& pixel: province->innerPixels)
		if ((pixel.x + pixel.y) % 8 == 0)
			strafedPixels.emplace_back(pixel);
}

void ImageCanvas::dismarkProvince(const std::shared_ptr<Province>& province) const
{
	// This fires when provinces within link are deselected, we're restoring their original color.
	// We're not removing pixels from shadedPixels as it's faster to drop them all and regenerate them.

	for (const auto& pixel: province->innerPixels)
	{
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
		imageData[offset] = province->r;
		imageData[offset + 1] = province->g;
		imageData[offset + 2] = province->b;
	}
}

void ImageCanvas::markProvince(const std::shared_ptr<Province>& province)
{
	// This fires when a province is marked into a link. It should go black if black mode is on.
	// This happens before shading is applied.

	// This is only relevant in black mode.
	if (!black)
		return;

	for (const auto& pixel: province->innerPixels)
	{
		shadedPixels.emplace_back(pixel);
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
		imageData[offset] = 0;
		imageData[offset + 1] = 0;
		imageData[offset + 2] = 0;
	}
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
			for (const auto& relevantProvince: getRelevantProvinces(activeLink))
				if (*relevantProvince == *province)
				{
					// Trigger deselect procedure.
					stageToggleProvinceByID(province->ID);
					event.Skip();
					return;
				}
		}

		// Case 2: SELECT LINK if province is linked elsewhere.
		for (const auto& link: *activeVersion->getLinks())
		{
			for (const auto& relevantProvince: getRelevantProvinces(link))
			{
				if (*relevantProvince == *province)
				{
					// trigger select link procedure.
					selectLink(link->getID());
					event.Skip();
					return;
				}
			}
		}

		// Case 3: SELECT PROVINCE since it's not linked anywhere.
		stageToggleProvinceByID(province->ID);
	}
	event.Skip();
}

void ImageCanvas::rightUp(wxMouseEvent& event)
{
	// Right up means deselect active link, nothing else.
	auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
	eventListener->QueueEvent(evt->Clone());
	event.Skip();
}

const std::vector<std::shared_ptr<Province>>& ImageCanvas::getRelevantProvinces(const std::shared_ptr<LinkMapping>& link) const
{
	if (selector == ImageTabSelector::SOURCE)
		return link->getSources();
	else // if (selector == ImageTabSelector::TARGET)
		return link->getTargets();
}


void ImageCanvas::selectLink(const int linkID) const
{
	auto* evt = new wxCommandEvent(wxEVT_SELECT_LINK_BY_ID);
	evt->SetInt(linkID);
	eventListener->QueueEvent(evt->Clone());
}

void ImageCanvas::stageToggleProvinceByID(const int provinceID) const
{
	wxCommandEvent evt(wxEVT_TOGGLE_PROVINCE);

	// We have a single Int as a common data field, so, be creative with least fuss.
	// Fortunately there are no provinces with ID 0 in any of PDX games.
	if (selector == ImageTabSelector::SOURCE)
		evt.SetInt(provinceID);
	else if (selector == ImageTabSelector::TARGET)
		evt.SetInt(-provinceID);

	// Notify authorities.
	eventListener->QueueEvent(evt.Clone());
}

void ImageCanvas::toggleProvinceByID(const int ID)
{
	if (!activeLink)
		return;

	// Do we have this province in our link?
	for (const auto& province: getRelevantProvinces(activeLink))
	{
		if (province->ID == ID)
		{
			// mark the province as black if needed.
			markProvince(province);
			// We need to add this province to strafe.
			strafeProvince(province);
			applyStrafedPixels();
			return;
		}
	}

	// This province was removed, so its previous pixels needs to go back to full color... But where are they...
	const auto& province = definitions->getProvinceForID(ID);
	if (province)
		dismarkProvince(province);

	// Easiest way about strafe change is to restrafe all, instead of hunting and pecking dismarked pixels.
	strafedPixels.clear();
	strafeProvinces();
	applyStrafedPixels();
}

wxPoint ImageCanvas::locateLinkCoordinates(int ID) const
{
	auto toReturn = wxPoint(0, 0);
	std::shared_ptr<LinkMapping> link = nullptr;
	// We're presumably operating on our own activeLink
	if (activeLink && activeLink->getID() == ID)
		link = activeLink;
	else
	{
		// But maybe not.
		for (const auto& otherLink: *activeVersion->getLinks())
			if (otherLink->getID() == ID)
			{
				link = otherLink;
				break;
			}
	}

	// find out first province's pixels.
	if (link)
	{
		const auto& relevantProvinces = getRelevantProvinces(link);
		if (!relevantProvinces.empty()) // not all links have provinces in them.
		{
			const auto& province = relevantProvinces.front();
			// provinces usually have pixels.
			if (!province->innerPixels.empty())
			{
				const auto& pixel = province->innerPixels.front();
				// And there we have it.
				toReturn.x = pixel.x;
				toReturn.y = pixel.y;
			}
		}
	}

	return toReturn;
}
