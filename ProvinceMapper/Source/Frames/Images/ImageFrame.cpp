#include "ImageFrame.h"
#include "ImageCanvas.h"
#include "OSCompatibilityLayer.h"
#include <wx/dcbuffer.h>
#include <wx/splitter.h>

ImageFrame::ImageFrame(wxWindow* parent,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* sourceImg,
	 wxImage* targetImg,
	 const std::shared_ptr<Definitions>& sourceDefs,
	 const std::shared_ptr<Definitions>& targetDefs):
	 wxFrame(parent, wxID_ANY, "Provinces", wxDefaultPosition, wxSize(1200, 800), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL),
	 eventHandler(parent)
{
	Bind(wxEVT_MENU, &ImageFrame::onToggleOrientation, this, wxID_REVERT);
	Bind(wxEVT_MENU, &ImageFrame::onToggleBlack, this, wxID_BOLD);
	Bind(wxEVT_CLOSE_WINDOW, &ImageFrame::onClose, this);
	Bind(wxEVT_REFRESH, &ImageFrame::onRefresh, this);

	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

	sourceCanvas = new ImageCanvas(splitter, ImageTabSelector::SOURCE, theActiveVersion, sourceImg, sourceDefs);
	targetCanvas = new ImageCanvas(splitter, ImageTabSelector::TARGET, theActiveVersion, targetImg, targetDefs);

	// We're setting scroll rate to minimum since we're not scrolling via mouse anyway, and it helps with automatic positioning.
	sourceCanvas->SetScrollRate(1, 1);
	sourceCanvas->SetVirtualSize(sourceCanvas->getWidth(), sourceCanvas->getHeight());
	sourceCanvas->SetBackgroundStyle(wxBG_STYLE_PAINT);
	sourceCanvas->Bind(wxEVT_PAINT, &ImageFrame::onScrollPaint, this);
	targetCanvas->SetScrollRate(1, 1);
	targetCanvas->SetVirtualSize(targetCanvas->getWidth(), targetCanvas->getHeight());
	targetCanvas->SetBackgroundStyle(wxBG_STYLE_PAINT);
	targetCanvas->Bind(wxEVT_PAINT, &ImageFrame::onScrollPaint, this);

	splitter->SetMinSize(wxSize(1200, 800));
	splitter->SetSashGravity(0.5);
	splitter->SplitVertically(sourceCanvas, targetCanvas);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void ImageFrame::onScrollPaint(wxPaintEvent& event)
{
	render();
}

void ImageFrame::onRefresh(wxCommandEvent& event)
{
	// force refresh comes from zooming. We need to store and then recalculate scroll position.
	auto refreshSource = false;
	auto refreshTarget = false;
	if (std::abs(sourceCanvas->getScale() - sourceCanvas->getOldScale()) > 0.001)
		refreshSource = true;
	if (std::abs(targetCanvas->getScale() - targetCanvas->getOldScale()) > 0.001)
		refreshTarget = true;

	const auto sourceHalfScreenX = static_cast<int>(sourceCanvas->GetScrollPageSize(wxHORIZONTAL) / 2.0);
	const auto sourceHalfScreenY = static_cast<int>(sourceCanvas->GetScrollPageSize(wxVERTICAL) / 2.0);
	const auto targetHalfScreenX = static_cast<int>(targetCanvas->GetScrollPageSize(wxHORIZONTAL) / 2.0);
	const auto targetHalfScreenY = static_cast<int>(targetCanvas->GetScrollPageSize(wxVERTICAL) / 2.0);
	const auto sourceScrollX = sourceCanvas->GetViewStart().x;
	const auto sourceScrollY = sourceCanvas->GetViewStart().y;
	const auto targetScrollX = targetCanvas->GetViewStart().x;
	const auto targetScrollY = targetCanvas->GetViewStart().y;
	const auto sourceCenterX = (sourceHalfScreenX + sourceScrollX) / sourceCanvas->getOldScale();
	const auto sourceCenterY = (sourceHalfScreenY + sourceScrollY) / sourceCanvas->getOldScale();
	const auto targetCenterX = (targetHalfScreenX + targetScrollX) / targetCanvas->getOldScale();
	const auto targetCenterY = (targetHalfScreenY + targetScrollY) / targetCanvas->getOldScale();

	render(); // render will change virtual size of the image thus setting scrollbars to random junk.

	const auto sourceUnits = wxPoint(static_cast<int>(sourceCenterX * sourceCanvas->getScale()), static_cast<int>(sourceCenterY * sourceCanvas->getScale()));
	const auto targetUnits = wxPoint(static_cast<int>(targetCenterX * targetCanvas->getScale()), static_cast<int>(targetCenterY * targetCanvas->getScale()));
	const auto sourceOffset = wxPoint(sourceUnits.x - sourceHalfScreenX, sourceUnits.y - sourceHalfScreenY);
	const auto targetOffset = wxPoint(targetUnits.x - targetHalfScreenX, targetUnits.y - targetHalfScreenY);
	if (refreshSource)
	{
		sourceCanvas->Scroll(sourceOffset);
		sourceCanvas->clearScale();
	}
	if (refreshTarget)
	{
		targetCanvas->Scroll(targetOffset);
		targetCanvas->clearScale();
	}

	Refresh();
}

void ImageFrame::render()
{
	auto newWidth = static_cast<int>(sourceCanvas->getWidth() * sourceCanvas->getScale());
	auto newHeight = static_cast<int>(sourceCanvas->getHeight() * sourceCanvas->getScale());
	sourceCanvas->SetVirtualSize(newWidth, newHeight);
	sourceCanvas->SetScale(sourceCanvas->getScale(), sourceCanvas->getScale());

	wxAutoBufferedPaintDC sourceDC(sourceCanvas);
	sourceCanvas->DoPrepareDC(sourceDC);
	sourceDC.Clear();
	const wxImage bmp(sourceCanvas->getWidth(), sourceCanvas->getHeight(), sourceCanvas->getImageData(), true);
	sourceDC.DrawBitmap(bmp, 0, 0);

	newWidth = static_cast<int>(targetCanvas->getWidth() * targetCanvas->getScale());
	newHeight = static_cast<int>(targetCanvas->getHeight() * targetCanvas->getScale());
	targetCanvas->SetVirtualSize(newWidth, newHeight);
	targetCanvas->SetScale(targetCanvas->getScale(), targetCanvas->getScale());

	wxAutoBufferedPaintDC targetDC(targetCanvas);
	targetCanvas->DoPrepareDC(targetDC);
	targetDC.Clear();
	const wxImage bmp2(targetCanvas->getWidth(), targetCanvas->getHeight(), targetCanvas->getImageData(), true);
	targetDC.DrawBitmap(bmp2, 0, 0);
}

void ImageFrame::onToggleOrientation(wxCommandEvent& event)
{
	if (splitter->GetSplitMode() == wxSPLIT_HORIZONTAL)
	{
		splitter->Unsplit();
		splitter->SplitVertically(sourceCanvas, targetCanvas);
		splitter->SetSashGravity(0.5);
	}
	else
	{
		splitter->Unsplit();
		splitter->SplitHorizontally(sourceCanvas, targetCanvas);
		splitter->SetSashGravity(0.5);
	}
}

void ImageFrame::onToggleBlack(wxCommandEvent& event)
{
	if (black == true)
	{
		black = false;
		sourceCanvas->clearBlack();
		targetCanvas->clearBlack();
		sourceCanvas->clearShadedPixels();
		sourceCanvas->restoreImageData();
		targetCanvas->clearShadedPixels();
		targetCanvas->restoreImageData();
		sourceCanvas->applyStrafedPixels();
		targetCanvas->applyStrafedPixels();
	}
	else
	{
		black = true;
		sourceCanvas->setBlack();
		targetCanvas->setBlack();
		sourceCanvas->generateShadedPixels();
		sourceCanvas->applyShadedPixels();
		targetCanvas->generateShadedPixels();
		targetCanvas->applyShadedPixels();
		sourceCanvas->applyStrafedPixels();
		targetCanvas->applyStrafedPixels();
	}
	render();
	Refresh();
}

void ImageFrame::onClose(wxCloseEvent& event)
{
	// We need to kill the app.
	eventHandler->QueueEvent(event.Clone());
}

void ImageFrame::activateLinkByIndex(const int row)
{
	sourceCanvas->activateLinkByIndex(row);
	targetCanvas->activateLinkByIndex(row);
	sourceCanvas->applyStrafedPixels();
	targetCanvas->applyStrafedPixels();
	render();
	Refresh();
}

void ImageFrame::activateLinkByID(const int ID)
{
	sourceCanvas->activateLinkByID(ID);
	targetCanvas->activateLinkByID(ID);
	sourceCanvas->applyStrafedPixels();
	targetCanvas->applyStrafedPixels();
	render();
	Refresh();
}

void ImageFrame::deactivateLink()
{
	sourceCanvas->deactivateLink();
	targetCanvas->deactivateLink();
	render();
	Refresh();
}

void ImageFrame::deleteActiveLink()
{
	sourceCanvas->deleteActiveLink();
	targetCanvas->deleteActiveLink();
	render();
	Refresh();
}

void ImageFrame::toggleProvinceByID(const int ID, const bool sourceImage)
{
	if (sourceImage)
		sourceCanvas->toggleProvinceByID(ID);
	else
		targetCanvas->toggleProvinceByID(ID);
	render();
	Refresh();
}

void ImageFrame::shadeProvinceByID(int ID, bool sourceImage)
{
	if (sourceImage)
		sourceCanvas->shadeProvinceByID(ID);
	else
		targetCanvas->shadeProvinceByID(ID);
	render();
	Refresh();
}

void ImageFrame::centerMap(int ID)
{
	const auto pt1 = sourceCanvas->locateLinkCoordinates(ID);
	const auto pt2 = targetCanvas->locateLinkCoordinates(ID);
	const auto scrollPageSizeX = sourceCanvas->GetScrollPageSize(wxHORIZONTAL);
	const auto scrollPageSizeY = sourceCanvas->GetScrollPageSize(wxVERTICAL);

	auto units = wxPoint(static_cast<int>(pt1.x * sourceCanvas->getScale()), static_cast<int>(pt1.y * sourceCanvas->getScale()));
	auto offset = wxPoint(units.x - scrollPageSizeX / 2, units.y - scrollPageSizeY / 2);
	sourceCanvas->Scroll(offset);

	units = wxPoint(static_cast<int>(pt2.x * targetCanvas->getScale()), static_cast<int>(pt2.y * targetCanvas->getScale()));
	offset = wxPoint(units.x - scrollPageSizeX / 2, units.y - scrollPageSizeY / 2);
	targetCanvas->Scroll(offset);

	render();
	Refresh();
}

void ImageFrame::setVersion(const std::shared_ptr<LinkMappingVersion>& version)
{
	sourceCanvas->setVersion(version);
	targetCanvas->setVersion(version);
	sourceCanvas->clearStrafedPixels();
	targetCanvas->clearStrafedPixels();
	sourceCanvas->restoreImageData();
	targetCanvas->restoreImageData();

	if (black == true)
	{
		sourceCanvas->generateShadedPixels();
		targetCanvas->generateShadedPixels();
		sourceCanvas->applyShadedPixels();
		targetCanvas->applyShadedPixels();
	}
	render();
	Refresh();
}
