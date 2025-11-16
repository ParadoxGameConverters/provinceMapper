#include "ImageCanvas.h"
#include "Frames/Links/DialogComment.h"
#include "Frames/Links/LinksTab.h"
#include "Frames/Links/ProvinceMappingsGrid.h"
#include "Frames/Links/TriangulationPairsGrid.h"
#include "Provinces/Province.h"
#include <Log.h>

wxDEFINE_EVENT(wxEVT_TOGGLE_PROVINCE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_LINK_BY_ID, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SELECT_TRIANGULATION_PAIR_BY_ID, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_REFRESH, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_MOUSE_AT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SCROLL_RELEASE_H, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SCROLL_RELEASE_V, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DELAUNAY_TRIANGULATE, wxCommandEvent);

ImageCanvas::ImageCanvas(wxWindow* parent,
	 ImageTabSelector theSelector,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* theImage,
	 std::shared_ptr<DefinitionsInterface> theDefinitions):
	 wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER | wxHSCROLL | wxVSCROLL), definitions(std::move(theDefinitions)),
	 eventHandler(parent)
{
	Bind(wxEVT_MOTION, &ImageCanvas::onMouseOver, this);
	Bind(wxEVT_LEFT_UP, &ImageCanvas::leftUp, this);
	Bind(wxEVT_RIGHT_UP, &ImageCanvas::rightUp, this);
	Bind(wxEVT_KEY_DOWN, &ImageCanvas::onKeyDown, this);
	Bind(wxEVT_MOUSEWHEEL, &ImageCanvas::onMouseWheel, this);
	Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &ImageCanvas::onScrollRelease, this);

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

void ImageCanvas::setImage(wxImage* newImage)
{
	if (!newImage || !newImage->IsOk())
		return;

	image = newImage;
	width = image->GetSize().GetX();
	height = image->GetSize().GetY();
	const auto newDataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(3);
	if (newDataSize != imageDataSize || !imageData)
	{
		delete[] imageData;
		imageData = new unsigned char[newDataSize];
	}
	imageDataSize = newDataSize;
	memcpy(imageData, image->GetData(), imageDataSize);
	SetVirtualSize(width, height);
	refreshStrafedPixelColors();
}

void ImageCanvas::activateLinkByIndex(const int row)
{
	if (activeVersion && row < static_cast<int>(activeVersion->getLinks()->size()))
	{
		lastClickedRow = row;
		// Strafe our provinces' pixels.
		strafeProvinces();
	}
}

void ImageCanvas::activateLinkByID(const int ID)
{
	if (!activeVersion)
		return;
	auto counter = 0;
	for (const auto& link: *activeVersion->getLinks())
	{
		if (link->getID() == ID)
		{
			lastClickedRow = counter;
			// Strafe our provinces' pixels.
			strafeProvinces();
			break;
		}
		++counter;
	}
}

void ImageCanvas::strafeProvinces()
{
	if (!activeVersion)
		return;

	const auto& activeLink = activeVersion->getActiveLink();
	if (!activeLink)
		return;

	for (const auto& province: getRelevantProvinces(activeLink))
		strafeProvince(province);
}

void ImageCanvas::strafeProvince(const std::shared_ptr<Province>& province)
{
	const auto* baseData = image->GetData();
	for (const auto& pixel: province->innerPixels)
		if ((pixel.x + pixel.y) % 8 == 0)
			{
				const auto offset = coordsToOffset(pixel.x, pixel.y, width);
				strafedPixels.emplace_back(pixel.x, pixel.y, baseData[offset], baseData[offset + 1], baseData[offset + 2]);
			}
}

void ImageCanvas::dismarkProvince(const std::shared_ptr<Province>& province) const
{
	// This fires when provinces within link are deselected, we're restoring their original color.
	// We're not removing pixels from shadedPixels as it's faster to drop them all and regenerate them.

	const auto* baseData = image->GetData();
	for (const auto& pixel: province->innerPixels)
	{
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
		imageData[offset] = baseData[offset];
		imageData[offset + 1] = baseData[offset + 1];
		imageData[offset + 2] = baseData[offset + 2];
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

void ImageCanvas::refreshStrafedPixelColors()
{
	if (strafedPixels.empty() || !image)
		return;

	const auto* baseData = image->GetData();
	const auto maxOffset = static_cast<int>(imageDataSize);
	for (auto& pixel: strafedPixels)
	{
		const auto offset = coordsToOffset(pixel.x, pixel.y, width);
		if (offset < 0 || offset + 2 >= maxOffset)
			continue;
		pixel.r = baseData[offset];
		pixel.g = baseData[offset + 1];
		pixel.b = baseData[offset + 2];
	}
}

void ImageCanvas::stageActivateTriangulationPairByID(const int ID) const
{
	auto* evt = new wxCommandEvent(wxEVT_SELECT_TRIANGULATION_PAIR_BY_ID);
	evt->SetInt(ID);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::onMouseOver(wxMouseEvent& event)
{
	auto x = CalcUnscrolledPosition(event.GetPosition()).x;
	auto y = CalcUnscrolledPosition(event.GetPosition()).y;
	x = static_cast<int>(x / scaleFactor);
	y = static_cast<int>(y / scaleFactor);

	// before we go on, if we have triangulation going on, notify the other canvas.
	if (triangulate)
	{
		wxCommandEvent evt(wxEVT_MOUSE_AT);
		// get creative, again.
		if (selector == ImageTabSelector::SOURCE)
			evt.SetId(x);
		else
			evt.SetId(-x);
		evt.SetInt(y);
		eventHandler->QueueEvent(evt.Clone());
	}

	// We may be out of scope if mouse leaves canvas.
	if (x >= 0 && x <= width - 1 && y >= 0 && y <= height - 1)
	{
		const auto name = nameAtCoords(wxPoint(x, y));
		this->SetToolTip(name);
		this->GetToolTip()->SetAutoPop(20000);
	}
	event.Skip();
}

void ImageCanvas::leftUp(const wxMouseEvent& event)
{
	// Left up means:
	// 1. select a mapping, or
	// 2. add a province to the existing mapping, or
	// 3. remove it from active mapping.
	// 4. special: if we're initing triangulation, we need raw points.
	// 5. special: if we're adding a permanent triangulation pair, add source/target point
	// 6. when triangulation mesh is enabled, and we clicked within the radius of a triangulation pair point, activate the pair.

	// What province have we clicked?
	auto x = CalcUnscrolledPosition(event.GetPosition()).x;
	auto y = CalcUnscrolledPosition(event.GetPosition()).y;
	x = static_cast<int>(x / scaleFactor);
	y = static_cast<int>(y / scaleFactor);
	const auto offs = coordsToOffset(x, y, width);
	// We may be out of scope if mouse leaves canvas.
	if (x >= 0 && x <= width - 1 && y >= 0 && y <= height - 1)
	{

		// case 6: enable triangulation pair if the user clicked within the radius of a point.
		if (showTriangulationMesh)
		{
			const auto pointRadius = static_cast<int>(std::round(5.0 / getScale()));
			if (selector == ImageTabSelector::SOURCE)
			{
				for (const auto& pair: *activeVersion->getTriangulationPairs())
				{
					const auto& sourcePoint = pair->getSourcePoint();
					if (!sourcePoint)
						continue;
					const auto clickDistToPoint = std::sqrt(std::pow(sourcePoint->x - x, 2) + std::pow(sourcePoint->y - y, 2));
					if (clickDistToPoint <= pointRadius)
					{
						stageActivateTriangulationPairByID(pair->getID());
						return;
					}
				}
			}
			else
			{
				for (const auto& pair: *activeVersion->getTriangulationPairs())
				{
					const auto& targetPoint = pair->getTargetPoint();
					if (!targetPoint)
						continue;
					const auto clickDistToPoint = std::sqrt(std::pow(targetPoint->x - x, 2) + std::pow(targetPoint->y - y, 2));
					if (clickDistToPoint <= pointRadius)
					{
						stageActivateTriangulationPairByID(pair->getID());
						return;
					}
				}
			}
		}

		// case 5: check if we're in the process of editing a triangulation pair
		if (const auto& activeTriangulationPair = getActiveTriangulationPair())
		{
			const auto point = wxPoint(x, y);
			if (selector == ImageTabSelector::SOURCE)
			{
				activeTriangulationPair->setSourcePoint(point);
			}
			else
			{
				activeTriangulationPair->setTargetPoint(point);
			}
			// Update the triangle generator.
			stageDelaunayTriangulate();

			stageTriangulationPairPointPlaced();
			stageRefresh();
			return;
		}

		const auto chroma = pixelPack(image->GetData()[offs], image->GetData()[offs + 1], image->GetData()[offs + 2]);
		// Province may not even be defined/instanced, let alone linked. Tread carefully.
		const auto province = definitions->getProvinceForChroma(chroma);
		if (!province || !activeVersion) // Do we have an active version? If not, we're in boo-boo. Same for undefined province.
		{
			// Play dead.
			return;
		}

		// Case 1: DESELECT PROVINCE if we have an active link with this province inside.
		const auto& activeLink = activeVersion->getActiveLink();
		if (activeLink)
		{
			for (const auto& relevantProvince: getRelevantProvinces(activeLink))
				if (*relevantProvince == *province)
				{
					// Trigger deselect procedure.
					stageToggleProvinceByID(province->ID);
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
					return;
				}
			}
		}

		// Case 3: SELECT PROVINCE since it's not linked anywhere, unless we're operating on a comment.
		if (!(activeLink && activeLink->getComment()))
			stageToggleProvinceByID(province->ID);
	}
}

void ImageCanvas::rightUp(wxMouseEvent& event)
{
	// Right up means deselect active link, which is serious stuff.
	// If our active link is dry, we're not deselecting it, we're deleting it.
	if (activeVersion->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_LINK);
		eventHandler->QueueEvent(evt->Clone());
	}
	if (getActiveTriangulationPair())
	{
		const auto* evt = new wxCommandEvent(wxEVT_DEACTIVATE_TRIANGULATION_PAIR);
		eventHandler->QueueEvent(evt->Clone());
	}
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
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageToggleProvinceByID(const std::string& provinceID) const
{
	wxCommandEvent evt(wxEVT_TOGGLE_PROVINCE);

	// We have a single Int as a common data field, so, be creative with least fuss.
	if (selector == ImageTabSelector::SOURCE)
		evt.SetString(provinceID);
	else if (selector == ImageTabSelector::TARGET)
		evt.SetString('-' + provinceID);

	// Notify authorities.
	eventHandler->QueueEvent(evt.Clone());
}

void ImageCanvas::toggleProvinceByID(const std::string& ID)
{
	if (!activeVersion)
		return;

	const auto& activeLink = activeVersion->getActiveLink();
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

void ImageCanvas::shadeProvinceByID(const std::string& ID)
{
	// this is called when we're marking a province outside of a working link. Often a preface to a new link being initialized.
	// Irrelevant unless we're shading.
	if (!black)
		return;

	const auto& province = definitions->getProvinceForID(ID);
	if (province)
		markProvince(province);
}

wxPoint ImageCanvas::locateLinkCoordinates(int ID) const
{
	auto toReturn = wxPoint(0, 0);
	std::shared_ptr<LinkMapping> link = nullptr;
	// We're presumably operating on the activeLink.
	const auto& activeLink = activeVersion->getActiveLink();
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

wxPoint ImageCanvas::locateProvinceCoordinates(const std::string& ID) const
{
	auto toReturn = wxPoint(0, 0);
	const auto& provinces = definitions->getProvinces();
	if (const auto& provinceItr = provinces.find(ID); provinceItr != provinces.end())
		for (const auto& pixel: provinceItr->second->innerPixels)
		{
			toReturn = wxPoint(pixel.x, pixel.y);
			break;
		}
	return toReturn;
}

void ImageCanvas::deleteActiveLink()
{
	const auto& activeLink = activeVersion->getActiveLink();
	if (activeLink)
	{
		// We need to restore full color to our provinces.
		for (const auto& province: getRelevantProvinces(activeLink))
			dismarkProvince(province);
		strafedPixels.clear();
	}
}

void ImageCanvas::onKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_F3:
			stageAddLink();
			break;
		case WXK_F4:
			// spawn a dialog to name the comment.
			stageAddComment();
			break;
		case WXK_F5:
			stageSave();
			break;
		case WXK_F6:
			stageAddTriangulationPair();
			break;
		case WXK_F7:
			stageAutogenerateMappings();
			break;
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
			stageDeleteLinkOrTriangulationPair();
			break;
		case WXK_NUMPAD_SUBTRACT:
			stageMoveUp();
			break;
		case WXK_NUMPAD_ADD:
			stageMoveDown();
			break;
		case WXK_NUMPAD_MULTIPLY:
			stageMoveVersionRight();
			break;
		case WXK_NUMPAD_DIVIDE:
			stageMoveVersionLeft();
			break;
		default:
			event.Skip();
	}
}

void ImageCanvas::stageAddComment()
{
	auto* dialog = new DialogComment(this, "Add Comment", lastClickedRow);
	dialog->ShowModal();
}

void ImageCanvas::stageDeleteLinkOrTriangulationPair() const
{
	// Do nothing unless working on active link. Don't want accidents here.
	if (activeVersion->getActiveTriangulationPair())
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_TRIANGULATION_PAIR);
		eventHandler->QueueEvent(evt->Clone());
	}
	if (activeVersion->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_DELETE_ACTIVE_LINK);
		eventHandler->QueueEvent(evt->Clone());
	}
}

void ImageCanvas::onMouseWheel(const wxMouseEvent& event)
{
	if (event.GetWheelRotation() != 0)
	{
		rotationDelta += event.GetWheelRotation();
		if (rotationDelta > 5)
		{
			rotationDelta = 0;
			zoomIn();
		}
		else if (rotationDelta < -5)
		{
			rotationDelta = 0;
			zoomOut();
		}
	}
}

void ImageCanvas::zoomIn()
{
	auto needRefresh = false;
	oldScaleFactor = scaleFactor;
	if (scaleFactor < 1)
	{
		scaleFactor += 0.1;
		needRefresh = true;
	}
	else if (scaleFactor < 3)
	{
		scaleFactor += 0.5;
		needRefresh = true;
	}
	else if (scaleFactor < 10)
	{
		scaleFactor += 1;
		needRefresh = true;
	}
	// else nothing. Not going over 10.

	if (needRefresh)
		stageRefresh();
}

void ImageCanvas::zoomOut()
{
	auto needRefresh = false;
	oldScaleFactor = scaleFactor;
	if (scaleFactor >= 4)
	{
		scaleFactor -= 1;
		needRefresh = true;
	}
	else if (scaleFactor >= 1.5)
	{
		scaleFactor -= 0.5;
		needRefresh = true;
	}
	else if (scaleFactor >= 0.2)
	{
		scaleFactor -= 0.1;
		needRefresh = true;
	}
	// not going below 0.1.

	if (needRefresh)
		stageRefresh();
}

void ImageCanvas::stageRefresh() const
{
	wxCommandEvent evt(wxEVT_REFRESH);
	if (selector == ImageTabSelector::SOURCE)
		evt.SetId(0);
	else if (selector == ImageTabSelector::TARGET)
		evt.SetId(1);
	eventHandler->QueueEvent(evt.Clone());
}

void ImageCanvas::stageTriangulationPairPointPlaced() const
{
	wxCommandEvent evt(wxEVT_REFRESH_ACTIVE_TRIANGULATION_PAIR);
	eventHandler->QueueEvent(evt.Clone());
}

void ImageCanvas::stageDelaunayTriangulate() const
{
	const wxCommandEvent evt(wxEVT_DELAUNAY_TRIANGULATE);
	eventHandler->QueueEvent(evt.Clone());
}

void ImageCanvas::stageMoveUp() const
{
	if (activeVersion && activeVersion->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_UP);
		eventHandler->QueueEvent(evt->Clone());
	}
}

void ImageCanvas::stageMoveDown() const
{
	if (activeVersion && activeVersion->getActiveLink())
	{
		const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_LINK_DOWN);
		eventHandler->QueueEvent(evt->Clone());
	}
}

void ImageCanvas::stageSave() const
{
	const auto* evt = new wxCommandEvent(wxEVT_SAVE_LINKS);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageAddLink() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_LINK);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageAddTriangulationPair() const
{
	const auto* evt = new wxCommandEvent(wxEVT_ADD_TRIANGULATION_PAIR);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageAutogenerateMappings() const
{
	const auto* evt = new wxCommandEvent(wxEVT_AUTOGENERATE_MAPPINGS);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageMoveVersionLeft() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_LEFT);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::stageMoveVersionRight() const
{
	const auto* evt = new wxCommandEvent(wxEVT_MOVE_ACTIVE_VERSION_RIGHT);
	eventHandler->QueueEvent(evt->Clone());
}

void ImageCanvas::pushZoomLevel(const int zoomLevel)
{
	oldScaleFactor = scaleFactor;
	scaleFactor = zoomLevel / 100.0;
}

void ImageCanvas::toggleTriangulate()
{
	if (triangulate)
	{
		triangulate = false;
	}
	else
	{
		triangulate = true;
	}
}

std::string ImageCanvas::nameAtCoords(const wxPoint& point)
{
	const auto offs = coordsToOffset(point.x, point.y, width);
	const auto chroma = pixelPack(image->GetData()[offs], image->GetData()[offs + 1], image->GetData()[offs + 2]);

	// cache?
	std::string name;
	if (tooltipCache.first == chroma)
		name = tooltipCache.second;
	else
	{
		// poke the definitions for a chroma name.
		const auto provID = definitions->getIDForChroma(chroma);
		if (provID)
			name = *provID + " - ";
		const auto& provinceName = definitions->getNameForChroma(chroma);
		if (provinceName)
			name += *provinceName;
		else
			name += "UNDEFINED";
		const auto& miscInfo = definitions->getMiscForChroma(chroma);
		if (miscInfo)
			name += *miscInfo;
		tooltipCache = std::pair(chroma, name);
	}
	return name;
}

std::shared_ptr<Province> ImageCanvas::provinceAtCoords(const wxPoint& point) const
{
	const auto offs = coordsToOffset(point.x, point.y, width);
	const auto chroma = pixelPack(image->GetData()[offs], image->GetData()[offs + 1], image->GetData()[offs + 2]);

	return definitions->getProvinceForChroma(chroma);
}

void ImageCanvas::onScrollRelease(wxScrollWinEvent& event)
{
	int select;
	if (selector == ImageTabSelector::SOURCE)
		select = 0;
	else
		select = 1;
	if (event.GetOrientation() == wxHORIZONTAL)
	{
		auto evt = wxCommandEvent(wxEVT_SCROLL_RELEASE_H);
		evt.SetId(select);
		eventHandler->AddPendingEvent(evt);
	}
	else
	{
		auto evt = wxCommandEvent(wxEVT_SCROLL_RELEASE_V);
		evt.SetId(select);
		eventHandler->AddPendingEvent(evt);
	}
	event.Skip();
}
