#include "ImageFrame.h"
#include "ImageCanvas.h"
#include "OSCompatibilityLayer.h"
#include "StatusBar.h"
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
	Bind(wxEVT_TOGGLE_TRIANGULATE, &ImageFrame::onTriangulate, this);
	Bind(wxEVT_POINT_PLACED, &ImageFrame::onPointPlaced, this);
	Bind(wxEVT_MOUSE_AT, &ImageFrame::triangulateAtPoint, this);

	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxEXPAND);

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

	statusBar = new StatusBar(this);
	statusBar->Show();

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));
}

void ImageFrame::onScrollPaint(wxPaintEvent& event)
{
	render();
}

void ImageFrame::onRefresh(wxCommandEvent& event)
{
	// force refresh comes from zooming. We need to store and then recalculate scroll position.
	if (event.GetId() == 0)
	{
		if (event.GetInt())
			sourceCanvas->pushZoomLevel(event.GetInt());
		else
			statusBar->setSourceZoom(static_cast<int>(sourceCanvas->getScale() * 100));
		const auto sourceHalfScreenX = static_cast<int>(sourceCanvas->GetScrollPageSize(wxHORIZONTAL) / 2.0);
		const auto sourceHalfScreenY = static_cast<int>(sourceCanvas->GetScrollPageSize(wxVERTICAL) / 2.0);
		const auto sourceScrollX = static_cast<double>(sourceCanvas->GetViewStart().x);
		const auto sourceScrollY = static_cast<double>(sourceCanvas->GetViewStart().y);
		const auto sourceCenterX = (sourceHalfScreenX + sourceScrollX) / sourceCanvas->getOldScale();
		const auto sourceCenterY = (sourceHalfScreenY + sourceScrollY) / sourceCanvas->getOldScale();

		renderSource(); // render will change virtual size of the image thus setting scrollbars to random junk.

		const auto sourceUnits = wxPoint(static_cast<int>(sourceCenterX * sourceCanvas->getScale()), static_cast<int>(sourceCenterY * sourceCanvas->getScale()));
		const auto sourceOffset = wxPoint(sourceUnits.x - sourceHalfScreenX, sourceUnits.y - sourceHalfScreenY);

		sourceCanvas->Scroll(sourceOffset);
		sourceCanvas->clearScale();
	}
	else if (event.GetId() == 1)
	{
		if (event.GetInt())
			targetCanvas->pushZoomLevel(event.GetInt());
		else
			statusBar->setTargetZoom(static_cast<int>(targetCanvas->getScale() * 100));
		const auto targetHalfScreenX = static_cast<int>(targetCanvas->GetScrollPageSize(wxHORIZONTAL) / 2.0);
		const auto targetHalfScreenY = static_cast<int>(targetCanvas->GetScrollPageSize(wxVERTICAL) / 2.0);
		const auto targetScrollX = static_cast<double>(targetCanvas->GetViewStart().x);
		const auto targetScrollY = static_cast<double>(targetCanvas->GetViewStart().y);
		const auto targetCenterX = (targetHalfScreenX + targetScrollX) / targetCanvas->getOldScale();
		const auto targetCenterY = (targetHalfScreenY + targetScrollY) / targetCanvas->getOldScale();

		renderTarget();

		const auto targetUnits = wxPoint(static_cast<int>(targetCenterX * targetCanvas->getScale()), static_cast<int>(targetCenterY * targetCanvas->getScale()));
		const auto targetOffset = wxPoint(targetUnits.x - targetHalfScreenX, targetUnits.y - targetHalfScreenY);
		targetCanvas->Scroll(targetOffset);
		targetCanvas->clearScale();
	}

	Refresh();
}

void ImageFrame::render() const
{
	renderSource();
	renderTarget();
}

void ImageFrame::renderSource() const
{
	const auto newWidth = static_cast<int>(sourceCanvas->getWidth() * sourceCanvas->getScale());
	const auto newHeight = static_cast<int>(sourceCanvas->getHeight() * sourceCanvas->getScale());
	sourceCanvas->SetVirtualSize(newWidth, newHeight);
	sourceCanvas->SetScale(sourceCanvas->getScale(), sourceCanvas->getScale());

	wxAutoBufferedPaintDC sourceDC(sourceCanvas);
	sourceCanvas->DoPrepareDC(sourceDC);
	sourceDC.Clear();
	const wxImage bmp(sourceCanvas->getWidth(), sourceCanvas->getHeight(), sourceCanvas->getImageData(), true);
	sourceDC.DrawBitmap(bmp, 0, 0);

	if (statusBar->isTriangulate())
	{
		wxPen pen = sourceDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / sourceCanvas->getScale())));
		sourceDC.SetPen(pen);
		sourceDC.SetBrush(*wxRED_BRUSH);
		// triangulation points
		for (const auto& point: sourceCanvas->getPoints())
		{
			sourceDC.DrawCircle(point, static_cast<int>(std::round(5.0 / sourceCanvas->getScale())));
		}
	}

	if (sourcePointer)
	{
		wxPen pen = sourceDC.GetPen();
		pen.SetColour("red");
		sourceDC.SetPen(pen);
		sourceDC.DrawLine(sourcePointer->x - static_cast<int>(std::round(20 / sourceCanvas->getScale())),
			 sourcePointer->y,
			 sourcePointer->x + static_cast<int>(std::round(20 / sourceCanvas->getScale())),
			 sourcePointer->y);
		sourceDC.DrawLine(sourcePointer->x,
			 sourcePointer->y - static_cast<int>(std::round(20 / sourceCanvas->getScale())),
			 sourcePointer->x,
			 sourcePointer->y + static_cast<int>(std::round(20 / sourceCanvas->getScale())));
		const auto name = sourceCanvas->nameAtCoords(*sourcePointer);
		sourceDC.SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
		sourceDC.SetTextForeground(*wxYELLOW);
		sourceDC.DrawText(name, sourcePointer->x + 10, sourcePointer->y + 10);
		sourceDC.SetBrush(*wxTRANSPARENT_BRUSH);
		sourceDC.DrawRectangle(sourceRect);
	}
}

void ImageFrame::renderTarget() const
{
	const auto newWidth = static_cast<int>(targetCanvas->getWidth() * targetCanvas->getScale());
	const auto newHeight = static_cast<int>(targetCanvas->getHeight() * targetCanvas->getScale());
	targetCanvas->SetVirtualSize(newWidth, newHeight);
	targetCanvas->SetScale(targetCanvas->getScale(), targetCanvas->getScale());

	wxAutoBufferedPaintDC targetDC(targetCanvas);
	targetCanvas->DoPrepareDC(targetDC);
	targetDC.Clear();
	const wxImage bmp2(targetCanvas->getWidth(), targetCanvas->getHeight(), targetCanvas->getImageData(), true);
	targetDC.DrawBitmap(bmp2, 0, 0);

	if (statusBar->isTriangulate())
	{
		wxPen pen = targetDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / targetCanvas->getScale())));
		targetDC.SetPen(pen);
		targetDC.SetBrush(*wxRED_BRUSH);
		// triangulation points
		for (const auto& point: targetCanvas->getPoints())
		{
			targetDC.DrawCircle(point, static_cast<int>(std::round(5.0 / targetCanvas->getScale())));
		}
	}

	if (targetPointer)
	{
		wxPen pen = targetDC.GetPen();
		pen.SetColour("red");
		targetDC.SetPen(pen);
		targetDC.DrawLine(targetPointer->x - static_cast<int>(std::round(20 / targetCanvas->getScale())),
			 targetPointer->y,
			 targetPointer->x + static_cast<int>(std::round(20 / targetCanvas->getScale())),
			 targetPointer->y);
		targetDC.DrawLine(targetPointer->x,
			 targetPointer->y - static_cast<int>(std::round(20 / targetCanvas->getScale())),
			 targetPointer->x,
			 targetPointer->y + static_cast<int>(std::round(20 / targetCanvas->getScale())));
		const auto name = targetCanvas->nameAtCoords(*targetPointer);
		targetDC.SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
		targetDC.SetTextForeground(*wxYELLOW);
		targetDC.DrawText(name, targetPointer->x + 10, targetPointer->y + 10);
		targetDC.SetBrush(*wxTRANSPARENT_BRUSH);
		targetDC.DrawRectangle(targetRect);
	}
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
	{
		sourceCanvas->toggleProvinceByID(ID);
		renderSource();
	}
	else
	{
		targetCanvas->toggleProvinceByID(ID);
		renderTarget();
	}
	Refresh();
}

void ImageFrame::shadeProvinceByID(int ID, bool sourceImage)
{
	if (sourceImage)
	{
		sourceCanvas->shadeProvinceByID(ID);
		renderSource();
	}
	else
	{
		targetCanvas->shadeProvinceByID(ID);
		renderTarget();
	}
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

void ImageFrame::showToolbar() const
{
	statusBar->Show();
}

void ImageFrame::onTriangulate(wxCommandEvent& event)
{
	sourceCanvas->toggleTriangulate();
	targetCanvas->toggleTriangulate();
	determineTriangulationSanity();
	render();
	Refresh();
}

void ImageFrame::onPointPlaced(wxCommandEvent& event)
{
	statusBar->setPointPlaced(event.GetInt());
	if (event.GetInt() <= 3)
		renderSource();
	else
		renderTarget();
	determineTriangulationSanity();
	Refresh();
}

void ImageFrame::determineTriangulationSanity()
{
	if (statusBar->isTriangulate() && sourceCanvas->getPoints().size() == 3 && targetCanvas->getPoints().size() == 3)
	{
		triangulationIsSane = true;
		statusBar->setTriangulationSane(true);
		buildBounds();
	}
	else
	{
		statusBar->setTriangulationSane(false);
		triangulationIsSane = false;
	}
}

void ImageFrame::buildBounds()
{
	// these bounds are the limits in which our triangulation will work.

	double maxX = -1;
	double minX = sourceCanvas->getWidth();
	double maxY = -1;
	double minY = sourceCanvas->getHeight();
	for (const auto& point: sourceCanvas->getPoints())
	{
		if (point.x < minX)
			minX = point.x;
		if (point.x > maxX)
			maxX = point.x;
		if (point.y < minY)
			minY = point.y;
		if (point.y > maxY)
			maxY = point.y;
	}
	// expand by 20% for convenience.
	if (minX - (maxX - minX) * 0.2 > 0)
		minX = minX - (maxX - minX) * 0.2;
	if (minY - (maxY - minY) * 0.2 > 0)
		minY = minY - (maxY - minY) * 0.2;
	if (maxX + (maxX - minX) * 0.2 < sourceCanvas->getWidth())
		maxX = maxX + (maxX - minX) * 0.2;
	if (maxY + (maxY - minY) * 0.2 < sourceCanvas->getHeight())
		maxY = maxY + (maxY - minY) * 0.2;
	minX = std::round(minX);
	minY = std::round(minY);
	maxX = std::round(maxX);
	maxY = std::round(maxY);
	sourceRect = wxRect(wxPoint(static_cast<int>(minX), static_cast<int>(minY)), wxPoint(static_cast<int>(maxX), static_cast<int>(maxY)));

	maxX = -1;
	minX = targetCanvas->getWidth();
	maxY = -1;
	minY = targetCanvas->getHeight();
	for (const auto& point: targetCanvas->getPoints())
	{
		if (point.x < minX)
			minX = point.x;
		if (point.x > maxX)
			maxX = point.x;
		if (point.y < minY)
			minY = point.y;
		if (point.y > maxY)
			maxY = point.y;
	}
	if (minX - (maxX - minX) * 0.2 > 0)
		minX = minX - (maxX - minX) * 0.2;
	if (minY - (maxY - minY) * 0.2 > 0)
		minY = minY - (maxY - minY) * 0.2;
	if (maxX + (maxX - minX) * 0.2 < targetCanvas->getWidth())
		maxX = maxX + (maxX - minX) * 0.2;
	if (maxY + (maxY - minY) * 0.2 < targetCanvas->getHeight())
		maxY = maxY + (maxY - minY) * 0.2;
	targetRect = wxRect(wxPoint(static_cast<int>(minX), static_cast<int>(minY)), wxPoint(static_cast<int>(maxX), static_cast<int>(maxY)));
}

void ImageFrame::triangulateAtPoint(wxCommandEvent& event)
{
	if (!triangulationIsSane)
		return;

	wxPoint currentPosition;
	if (event.GetId() < 0)
		currentPosition.x = -event.GetId();
	else
		currentPosition.x = event.GetId();
	currentPosition.y = event.GetInt();

	if (event.GetId() < 0) // this comes from target canvas.
	{
		if (targetRect.Contains(currentPosition))
		{
			sourcePointer = triangulate(targetCanvas->getPoints(), sourceCanvas->getPoints(), sourceRect, currentPosition);
			renderSource();
		}
		else
		{
			sourcePointer.reset();
		}
	}
	else
	{
		if (sourceRect.Contains(currentPosition))
		{
			targetPointer = triangulate(sourceCanvas->getPoints(), targetCanvas->getPoints(), targetRect, currentPosition);
			renderTarget();
		}
		else
		{
			targetPointer.reset();
		}
	}
	Refresh();
}

double ImageFrame::pointDistance(const wxPoint& point1, const wxPoint& point2)
{
	const auto sum = std::pow(point1.x - point2.x, 2) + std::pow(point1.y - point2.y, 2);
	return std::sqrt(sum);
}

wxPoint ImageFrame::triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxRect& targetRect, const wxPoint& sourcePoint)
{
	// move the source point in reference to the source origin
	auto movedSource = sourcePoint - sources[0];

	// construct a basis matrix for the source triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	float sourceA = sources[1].x - sources[0].x;
	float sourceB = sources[2].x - sources[0].x;
	float sourceC = sources[1].y - sources[0].y;
	float sourceD = sources[2].y - sources[0].y;

	// construct the inverse of the source basis matrix:
	// ___1___ ( d -b )
	// ad - bc (-c  a )
	auto sourceDeterminant = 1 / (sourceA * sourceD - sourceB * sourceC);
	auto inverseA = sourceDeterminant *  sourceD;
	auto inverseB = sourceDeterminant * -sourceB;
	auto inverseC = sourceDeterminant * -sourceC;
	auto inverseD = sourceDeterminant *  sourceA;

	// transform the source point into the source triangle basis
	auto sourceU = movedSource.x * inverseA + movedSource.y * inverseB;
	auto sourceV = movedSource.x * inverseC + movedSource.y * inverseD;

	// silently move from source triangle basis to destination triangle basis
	auto targetU = sourceU;
	auto targetV = sourceV;

	// construct a basis matrix for the target triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	float targetA = targets[1].x - targets[0].x;
	float targetB = targets[2].x - targets[0].x;
	float targetC = targets[1].y - targets[0].y;
	float targetD = targets[2].y - targets[0].y;

	// transform the target point from the destination triangle basis
	wxPoint target;
	target.x = targetU * targetA + targetV * targetB;
	target.y = targetU * targetC + targetV * targetD;

	// move the target point in reference to the source origin
	return target + targets[0];
}