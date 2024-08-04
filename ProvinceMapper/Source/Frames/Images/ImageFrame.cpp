#include "ImageFrame.h"
#include "Configuration/Configuration.h"
#include "ImageCanvas.h"
#include "OSCompatibilityLayer.h"
#include "StatusBar.h"
#include "LinkMapper/TriangulationPointPair.h"
#include <wx/dcbuffer.h>
#include <wx/splitter.h>


using Delaunay = tpp::Delaunay;

ImageFrame::ImageFrame(wxWindow* parent,
	 const wxPoint& position,
	 const wxSize& size,
	 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
	 wxImage* sourceImg,
	 wxImage* targetImg,
	 const std::shared_ptr<DefinitionsInterface>& sourceDefs,
	 const std::shared_ptr<DefinitionsInterface>& targetDefs,
	 std::shared_ptr<Configuration> theConfiguration):
	 wxFrame(parent, wxID_ANY, "Provinces", position, size, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL),
	 configuration(std::move(theConfiguration)), eventHandler(parent)
{
	Bind(wxEVT_MENU, &ImageFrame::onToggleOrientation, this, wxID_REVERT);
	Bind(wxEVT_MENU, &ImageFrame::onToggleBlack, this, wxID_BOLD);
	Bind(wxEVT_CLOSE_WINDOW, &ImageFrame::onClose, this);
	Bind(wxEVT_REFRESH, &ImageFrame::onRefresh, this);
	Bind(wxEVT_TOGGLE_TRIANGULATE, &ImageFrame::onTriangulate, this);
	Bind(wxEVT_POINT_PLACED, &ImageFrame::onPointPlaced, this);
	Bind(wxEVT_MOUSE_AT, &ImageFrame::triangulateAtPoint, this);
	Bind(wxEVT_SIZE, &ImageFrame::onResize, this);
	Bind(wxEVT_MOVE, &ImageFrame::onMove, this);
	Bind(wxEVT_SCROLL_RELEASE_H, &ImageFrame::onScrollReleaseH, this);
	Bind(wxEVT_SCROLL_RELEASE_V, &ImageFrame::onScrollReleaseV, this);
	Bind(wxEVT_LOCK, &ImageFrame::onLock, this);

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

	auto sbPosition = wxDefaultPosition;
	if (configuration->getStatusBarPos())
		sbPosition = wxPoint(configuration->getStatusBarPos()->x, configuration->getStatusBarPos()->y);

	SetIcon(wxIcon(wxT("converter.ico"), wxBITMAP_TYPE_ICO, 16, 16));

	statusBar = new StatusBar(this, sbPosition, configuration);
	if (configuration->isStatusBarOn())
		statusBar->Show();
}

void ImageFrame::onScrollPaint(wxPaintEvent& event)
{
	render();
}

void ImageFrame::onRefresh(const wxCommandEvent& event)
{
	// force refresh comes from zooming. We need to store and then recalculate scroll position.
	if (event.GetId() == 0)
	{
		if (event.GetInt())
			sourceCanvas->pushZoomLevel(event.GetInt());
		else
			statusBar->setSourceZoom(static_cast<int>(sourceCanvas->getScale() * 100));
	}
	else
	{
		if (event.GetInt())
			targetCanvas->pushZoomLevel(event.GetInt());
		else
			statusBar->setTargetZoom(static_cast<int>(targetCanvas->getScale() * 100));
	}

	if (lock)
	{
		if (event.GetId() == 0)
		{
			// source canvas is scaling target.
			const auto scaleFactor = sourceCanvas->getScale() / sourceCanvas->getOldScale();
			const auto newScale = static_cast<int>(std::round(targetCanvas->getScale() * scaleFactor * 100));
			targetCanvas->pushZoomLevel(newScale);
			statusBar->setTargetZoom(newScale);
		}
		else
		{
			// target canvas is scaling source.
			const auto scaleFactor = targetCanvas->getScale() / targetCanvas->getOldScale();
			const auto newScale = static_cast<int>(std::round(sourceCanvas->getScale() * scaleFactor * 100));
			sourceCanvas->pushZoomLevel(newScale);
			statusBar->setSourceZoom(newScale);
		}
	}

	// with one or both scale factors set, we can start zooming.

	if (event.GetId() == 0 || lock)
	{
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
	if (event.GetId() == 1 || lock)
	{
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

	renderTriangulationMesh(sourceDC, true); // TODO: add a toggle for this, like for the shade

	// Draw all the triangulation pair points.
	for (const auto& pair: *sourceCanvas->getTriangulationPairs())
	{
		if (!pair->getSourcePoint())
		{
			continue;
		}
		wxPen pen = sourceDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / sourceCanvas->getScale())));
		sourceDC.SetPen(pen);
		sourceDC.SetBrush(*wxGREY_BRUSH);
		sourceDC.DrawCircle(*pair->getSourcePoint(), static_cast<int>(std::round(5.0 / sourceCanvas->getScale())));
	}

	// Draw the active triangulation pair point with a different colour.
	const auto& activeTriangulationPair = sourceCanvas->getActiveTriangulationPair();
	if (activeTriangulationPair && activeTriangulationPair->getSourcePoint())
	{
		wxPen pen = sourceDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / sourceCanvas->getScale())));
		sourceDC.SetPen(pen);
		sourceDC.SetBrush(*wxBLUE_BRUSH); // blue instead of red, to differentiate from the old 3 triangulatin points per canvas
		sourceDC.DrawCircle(*activeTriangulationPair->getSourcePoint(), static_cast<int>(std::round(5.0 / sourceCanvas->getScale())));
	}

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
	
	renderTriangulationMesh(targetDC, false); // TODO: add a toggle for this, like for the shade

	// Draw all the triangulation pair points.
	for (const auto& pair: *targetCanvas->getTriangulationPairs())
	{
		if (!pair->getTargetPoint())
		{
			continue;
		}
		wxPen pen = targetDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / targetCanvas->getScale())));
		targetDC.SetPen(pen);
		targetDC.SetBrush(*wxGREY_BRUSH);
		targetDC.DrawCircle(*pair->getTargetPoint(), static_cast<int>(std::round(5.0 / targetCanvas->getScale())));
	}

	// Draw the active triangulation pair point with a different colour.
	const auto& activeTriangulationPair = targetCanvas->getActiveTriangulationPair();
	if (activeTriangulationPair && activeTriangulationPair->getTargetPoint())
	{
		wxPen pen = targetDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / targetCanvas->getScale())));
		targetDC.SetPen(pen);
		targetDC.SetBrush(*wxBLUE_BRUSH); // blue instead of red, to differentiate from the old 3 triangulatin points per canvas
		targetDC.DrawCircle(*activeTriangulationPair->getTargetPoint(), static_cast<int>(std::round(5.0 / targetCanvas->getScale())));
	}

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

void ImageFrame::onClose(const wxCloseEvent& event)
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

void ImageFrame::activateTriangulationPairByIndex(int row)
{
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

void ImageFrame::deactivateTriangulationPair()
{
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

void ImageFrame::deleteActiveTriangulationPair()
{
	// Re-render will cause the triangulation pair's points to disappear.
	render();
	Refresh();
}

void ImageFrame::activateTriangulationPairByID(const int ID)
{
	// Re-render will cause the triangulation pair's points to appear.
	render();
	Refresh();
}

void ImageFrame::toggleProvinceByID(const std::string& ID, const bool sourceImage)
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

void ImageFrame::shadeProvinceByID(const std::string& ID, bool sourceImage)
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
	const auto sourceScrollPageSizeX = sourceCanvas->GetScrollPageSize(wxHORIZONTAL);
	const auto sourceScrollPageSizeY = sourceCanvas->GetScrollPageSize(wxVERTICAL);
	const auto targetScrollPageSizeX = targetCanvas->GetScrollPageSize(wxHORIZONTAL);
	const auto targetScrollPageSizeY = targetCanvas->GetScrollPageSize(wxVERTICAL);

	auto units = wxPoint(static_cast<int>(pt1.x * sourceCanvas->getScale()), static_cast<int>(pt1.y * sourceCanvas->getScale()));
	auto offset = wxPoint(units.x - sourceScrollPageSizeX / 2, units.y - sourceScrollPageSizeY / 2);
	sourceCanvas->Scroll(offset);

	units = wxPoint(static_cast<int>(pt2.x * targetCanvas->getScale()), static_cast<int>(pt2.y * targetCanvas->getScale()));
	offset = wxPoint(units.x - targetScrollPageSizeX / 2, units.y - targetScrollPageSizeY / 2);
	targetCanvas->Scroll(offset);

	render();
	Refresh();
}

void ImageFrame::centerProvince(ImageTabSelector selector, const std::string& ID)
{
	if (selector == ImageTabSelector::SOURCE)
	{
		const auto pt = sourceCanvas->locateProvinceCoordinates(ID);
		const auto sourceScrollPageSizeX = sourceCanvas->GetScrollPageSize(wxHORIZONTAL);
		const auto sourceScrollPageSizeY = sourceCanvas->GetScrollPageSize(wxVERTICAL);
		const auto units = wxPoint(static_cast<int>(pt.x * sourceCanvas->getScale()), static_cast<int>(pt.y * sourceCanvas->getScale()));
		const auto offset = wxPoint(units.x - sourceScrollPageSizeX / 2, units.y - sourceScrollPageSizeY / 2);
		sourceCanvas->Scroll(offset);
		renderSource();
	}
	else
	{
		const auto pt = targetCanvas->locateProvinceCoordinates(ID);
		const auto targetScrollPageSizeX = targetCanvas->GetScrollPageSize(wxHORIZONTAL);
		const auto targetScrollPageSizeY = targetCanvas->GetScrollPageSize(wxVERTICAL);
		const auto units = wxPoint(static_cast<int>(pt.x * targetCanvas->getScale()), static_cast<int>(pt.y * targetCanvas->getScale()));
		const auto offset = wxPoint(units.x - targetScrollPageSizeX / 2, units.y - targetScrollPageSizeY / 2);
		targetCanvas->Scroll(offset);
		renderTarget();
	}
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
	configuration->setStatusBarOn(true);
	configuration->save();
	statusBar->Show();
}

double getDistanceFromCornerToLine(const wxPoint& linePoint1, const wxPoint& linePoint2, const wxPoint& cornerPoint)
{
	const double x1 = linePoint1.x;
   const double y1 = linePoint1.y;
   const double x2 = linePoint2.x;
   const double y2 = linePoint2.y;

	// Get the A, B, and C values for the equation of the line.
	const double A = y2 - y1;
	const double B = x1 - x2;
	const double C = x2 * y1 - x1 * y2;

   // Calculate the distance between the corner point and the line.
   const double dist = std::abs(A * cornerPoint.x + B * cornerPoint.y + C) / std::sqrt(A * A + B * B);
   return dist;
}

wxPoint calculateCoordinates(double a, double b, double x1, double y1, double d) // TODO: verify in practice if this is correct
{
	// Magnitude of the direction vector (b, -a)
	double magnitude = std::sqrt(b * b + a * a);

	// Unit vector in the direction of (b, -a)
	double unitX = b / magnitude;
	double unitY = -a / magnitude;

	// First point (x1 + d * unitX, y1 + d * unitY)
	const double X1 = x1 + d * unitX;
	const double Y1 = y1 + d * unitY;

	return wxPoint(X1, Y1);
}

void ImageFrame::delaunayTriangulate() // TODO: call this when a triangulation pair is added or removed
{
	triangles.clear();

	// We need to have at least 3 point pairs to triangulate.
	std::vector<std::shared_ptr<TriangulationPointPair>> validPairs;

	for (const auto& pair: sourceCanvas->getTriangulationPairs())
	{
		// A pair must have both a source and a target point.
		if (!pair->getSourcePoint() || !pair->getTargetPoint())
		{
			continue;
		}

		validPairs.push_back(pair);
	}

	if (validPairs.size() < 3)
	{
		Log(LogLevel::Info) << "Cannot triangulate with less than 3 point pairs.";
		return;
	}

	// Create a virtual point pair for each map corner.
	const auto& sourceMapWidth = sourceCanvas->getWidth();
   const auto& sourceMapHeight = sourceCanvas->getHeight();

   wxPoint topLeftPoint(0, 0);
   wxPoint topRightPoint(sourceMapWidth, 0); // TODO: check if x shouldn't be sourceMapWidth - 1
   wxPoint bottomLeftPoint(0, sourceMapHeight); // TODO: check if y shouldn't be sourceMapHeight - 1
   wxPoint bottomRightPoint(sourceMapWidth, sourceMapHeight);

   std::vector cornerPoints = {topLeftPoint, topRightPoint, bottomLeftPoint, bottomRightPoint};

   // For each corner, find 2 closest points and calculate coefficients for the linear equation.
	for (const auto& cornerPoint : cornerPoints)
	{
		std::optional<std::shared_ptr<TriangulationPointPair>> closestPair1;
		std::optional<std::shared_ptr<TriangulationPointPair>> closestPair2;

		// Calculate the closest 2 points as 2 points with the smallest distance to the corner.
		for (const auto& pair: validPairs)
		{
			const auto& sourcePoint = pair->getSourcePoint();
			const auto& distance = std::sqrt(std::pow(cornerPoint.x - sourcePoint->x, 2) + std::pow(cornerPoint.y - sourcePoint->y, 2));

			if (!closestPair1 || distance < std::sqrt(std::pow(cornerPoint.x - (*closestPair1)->getSourcePoint()->x, 2) + std::pow(cornerPoint.y - (*closestPair1)->getSourcePoint()->y, 2)))
			{
				closestPair2 = closestPair1;
				closestPair1 = pair;
			}
			else if (!closestPair2 || distance < std::sqrt(std::pow(cornerPoint.x - (*closestPair2)->getSourcePoint()->x, 2) + std::pow(cornerPoint.y - (*closestPair2)->getSourcePoint()->y, 2)))
			{
				closestPair2 = pair;
			}
		}

	  const auto& closestPoint1 = (*closestPair1)->getSourcePoint();
	  const auto& closestPoint2 = (*closestPair2)->getSourcePoint();
	  // closestPoint1 and closestPoint2 form the line.

		double srcLineA = static_cast<double>(closestPoint1->y - closestPoint2->y) / (closestPoint1->x - closestPoint2->x);
	  double srcLineB = static_cast<double>(closestPoint1->y) - srcLineA * closestPoint1->x;

		// Calculate the straight perpendicular to the line.
		double srcAPerpendicular = -1 / srcLineA;
	  double bPerpendicular = static_cast<double>(cornerPoint.y) - srcAPerpendicular * cornerPoint.x; // TODO: check if this is correct

		// Calculate the distance between the corner point and the line.
		double cornerDist = getDistanceFromCornerToLine(*closestPoint1, *closestPoint2, cornerPoint);

	  // Calculate the intersection point.
	  double intersectionX = (bPerpendicular - srcLineB) / (srcLineA - srcAPerpendicular);
		double intersectionY = srcLineA * intersectionX + srcLineB;

	  // Calculate the distance between the two closest points.
	  double closestPointsDist = std::sqrt(std::pow(closestPoint1->x - closestPoint2->x, 2) + std::pow(closestPoint1->y - closestPoint2->y, 2));

	  // Calculate the distance between the intersection point and closestPoint1.
	  double intersectionDistFromPoint1 = std::sqrt(std::pow(intersectionX - closestPoint1->x, 2) + std::pow(intersectionY - closestPoint1->y, 2));


	  // NOW WE CALCULATE THE EQUIVALENTS FOR THE TARGET MAP

	  // Calculate the linear equation for the target map's closest points pair.
	  double tgtLineA = static_cast<double>((*closestPair1)->getTargetPoint()->y - (*closestPair2)->getTargetPoint()->y) / ((*closestPair1)->getTargetPoint()->x - (*closestPair2)->getTargetPoint()->x);
	  double tgtLineB = static_cast<double>((*closestPair1)->getTargetPoint()->y) - tgtLineA * (*closestPair1)->getTargetPoint()->x;

	  // Calculate the distance between the two closest points on the target map.
	  double tgtClosestPointsDist = std::sqrt(std::pow((*closestPair1)->getTargetPoint()->x - (*closestPair2)->getTargetPoint()->x, 2) + std::pow((*closestPair1)->getTargetPoint()->y - (*closestPair2)->getTargetPoint()->y, 2));

	  // Calculate the distance between the intersection point and closestPoint1 on the target map, using the same ratio as on the source map.
	  double tgtIntersectionDistFromPoint1 = intersectionDistFromPoint1 * tgtClosestPointsDist / closestPointsDist;

	  // Find the intersection point on the target map.
	  wxPoint tgtIntersectionPoint = calculateCoordinates(tgtLineA, tgtLineB, (*closestPair1)->getTargetPoint()->x, (*closestPair1)->getTargetPoint()->y, tgtIntersectionDistFromPoint1);

	  // Calculate the line perpendicular to the target map's tgtLine, passing through tgtIntersectionPoint.
	  double tgtAPerpendicular = -1 / tgtLineA;
	  double tgtBPerpendicular = static_cast<double>(tgtIntersectionPoint.y) - tgtAPerpendicular * tgtIntersectionPoint.x;

	  // Scale cornerDist to the target map.
	  double tgtCornerDist = cornerDist * tgtClosestPointsDist / closestPointsDist;

	  // Calculate tgtCornerPoint as the point on the perpendicular line that is tgtCornerDist away from tgtIntersectionPoint.
	  double tgtCornerX = (tgtBPerpendicular - tgtLineB ) / (tgtLineA - tgtAPerpendicular); // TODO: check if this is correct
	  double tgtCornerY = tgtLineA * tgtCornerX + tgtLineB;

	  wxPoint tgtCornerPoint(tgtCornerX, tgtCornerY); // TODO: check if this is correct

	  int idToUse;
	  std::set<int> usedIds;
	  for (const auto& pair: *sourceCanvas->getTriangulationPairs())
	  {
		  usedIds.insert(pair->getID());
	  }
	  // Find an ID of TriangulationPointPair that's not used.
	  for (int i = 0; i < INT_MAX; i++)
	  {
		  if (!usedIds.contains(i))
		  {
			  idToUse = i;
			  break;
		  }
	  }
	  auto cornerPair = std::make_shared<TriangulationPointPair>(idToUse);
	  cornerPair->setSourcePoint(cornerPoint);
	  cornerPair->setTargetPoint(tgtCornerPoint);

	  validPairs.push_back(cornerPair);
	  




		//double bPerpendicular = static_cast<double>(cornerPoint.y) - aPerpendicular * cornerPoint.x;

	 // 	// Calculate the intersection point.
	 //  double x = (bPerpendicular - b) / (a - aPerpendicular);
		//double y = a * x + b;

	 // // Calculate the distance between the two closest points.
	 // double distBetweenClosestPoints = std::sqrt(std::pow(closestPoint1->x - closestPoint2->x, 2) + std::pow(closestPoint1->y - closestPoint2->y, 2));

		//// Calculate the distance between the intersection point and closestPoint1.
	 // double distToX = std::sqrt(std::pow(x - closestPoint1->x, 2) + std::pow(y - closestPoint1->y, 2));

	 // // Calculate the distance between the intersection point and the corner.
	 // double cornerDist = std::sqrt(std::pow(x - cornerPoint.x, 2) + std::pow(y - cornerPoint.y, 2));

	 // // Now translate the source map corner point to a target map point.
	 // // First, calculate the linear equation for the target map's closest points pair.
	 // double tgtA = static_cast<double>((*closestPair1)->getTargetPoint()->y - (*closestPair2)->getTargetPoint()->y) / ((*closestPair1)->getTargetPoint()->x - (*closestPair2)->getTargetPoint()->x);
	 // double tgtB = static_cast<double>((*closestPair1)->getTargetPoint()->y) - tgtA * (*closestPair1)->getTargetPoint()->x;

	 // // Calculate the distance between the two closest points on the target map.
	 // double tgtDistBetweenClosestPoints = std::sqrt(std::pow((*closestPair1)->getTargetPoint()->x - (*closestPair2)->getTargetPoint()->x, 2) + std::pow((*closestPair1)->getTargetPoint()->y - (*closestPair2)->getTargetPoint()->y, 2));


	 // 
	 // // Scale the distance from the target's map closestPair1 point to the corner.
	 // double tgtCornerDist = cornerDist * tgtDistBetweenClosestPoints / distBetweenClosestPoints;

	}




	std::map<std::pair<int, int>, std::shared_ptr<TriangulationPointPair>> pointToPairMap;

	std::vector<Delaunay::Point> delaunaySourceInput;
	for (const auto& pair: validPairs)
	{
		const auto& sourcePoint = pair->getSourcePoint();
		pointToPairMap[std::make_pair(sourcePoint->x, sourcePoint->y)] = pair;

		delaunaySourceInput.emplace_back(sourcePoint->x, sourcePoint->y);
	}

	// Use standard (non-constrained) Delaunay triangulation.
	Delaunay sourceTriangulator(delaunaySourceInput);
	sourceTriangulator.Triangulate();

	for (const auto& t: sourceTriangulator.faces())
	{
		// Get triangle's vertices.
		const auto& vertex1 = delaunaySourceInput[t.Org()];
		const auto& vertex2 = delaunaySourceInput[t.Dest()];
		const auto& vertex3 = delaunaySourceInput[t.Apex()];

		auto intPair1 = std::make_pair(static_cast<int>(vertex1[0]), static_cast<int>(vertex1[1]));
		auto intPair2 = std::make_pair(static_cast<int>(vertex2[0]), static_cast<int>(vertex2[1]));
		auto intPair3 = std::make_pair(static_cast<int>(vertex3[0]), static_cast<int>(vertex3[1]));

		const auto& pair1 = pointToPairMap[intPair1];
		const auto& pair2 = pointToPairMap[intPair2];
		const auto& pair3 = pointToPairMap[intPair3];

		Triangle triangle(pair1, pair2, pair3);

		triangles.push_back(triangle);
	}
}

void ImageFrame::renderTriangulationMesh(wxAutoBufferedPaintDC& paintDC, bool isSourceMap) const
{
   if (triangles.empty())
	{
	   return;
	}

	wxPen pen = paintDC.GetPen();
	pen.SetColour("red");
	paintDC.SetPen(pen);

	// iterate over triangles
	for (const auto& [pair1, pair2, pair3]: triangles)
	{

		// Draw the triangle.
		if (isSourceMap)
		{
			const auto& srcPoint1 = pair1->getSourcePoint();
			const auto& srcPoint2 = pair2->getSourcePoint();
			const auto& srcPoint3 = pair3->getSourcePoint();
			paintDC.DrawLine(srcPoint1->x, srcPoint1->y, srcPoint2->x, srcPoint2->y);
			paintDC.DrawLine(srcPoint2->x, srcPoint2->y, srcPoint3->x, srcPoint3->y);
			paintDC.DrawLine(srcPoint3->x, srcPoint3->y, srcPoint1->x, srcPoint1->y);
		}
	  else
	  {
		  const auto& tgtPoint1 = pair1->getTargetPoint();
			const auto& tgtPoint2 = pair2->getTargetPoint();
	   	const auto& tgtPoint3 = pair3->getTargetPoint();

		  paintDC.DrawLine(tgtPoint1->x, tgtPoint1->y, tgtPoint2->x, tgtPoint2->y);
		  paintDC.DrawLine(tgtPoint2->x, tgtPoint2->y, tgtPoint3->x, tgtPoint3->y);
		  paintDC.DrawLine(tgtPoint3->x, tgtPoint3->y, tgtPoint1->x, tgtPoint1->y);

	  }
	}
}

void ImageFrame::onTriangulate(wxCommandEvent& event)
{
	sourceCanvas->toggleTriangulate();
	targetCanvas->toggleTriangulate();
	determineTriangulationSanity();
	render();
	Refresh();
}

void ImageFrame::onPointPlaced(const wxCommandEvent& event)
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
	// expand by 50% for convenience.
	if (minX - (maxX - minX) * 0.5 > 0)
		minX = minX - (maxX - minX) * 0.5;
	if (minY - (maxY - minY) * 0.5 > 0)
		minY = minY - (maxY - minY) * 0.5;
	if (maxX + (maxX - minX) * 0.5 < sourceCanvas->getWidth())
		maxX = maxX + (maxX - minX) * 0.5;
	if (maxY + (maxY - minY) * 0.5 < sourceCanvas->getHeight())
		maxY = maxY + (maxY - minY) * 0.5;
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
	if (minX - (maxX - minX) * 0.5 > 0)
		minX = minX - (maxX - minX) * 0.5;
	if (minY - (maxY - minY) * 0.5 > 0)
		minY = minY - (maxY - minY) * 0.5;
	if (maxX + (maxX - minX) * 0.5 < targetCanvas->getWidth())
		maxX = maxX + (maxX - minX) * 0.5;
	if (maxY + (maxY - minY) * 0.5 < targetCanvas->getHeight())
		maxY = maxY + (maxY - minY) * 0.5;
	targetRect = wxRect(wxPoint(static_cast<int>(minX), static_cast<int>(minY)), wxPoint(static_cast<int>(maxX), static_cast<int>(maxY)));
}

void ImageFrame::triangulateAtPoint(const wxCommandEvent& event)
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
			sourcePointer = triangulate(targetCanvas->getPoints(), sourceCanvas->getPoints(), currentPosition);
			sourcePointer->x = std::min(std::max(sourcePointer->x, 20), sourceCanvas->getWidth() - 20);
			sourcePointer->y = std::min(std::max(sourcePointer->y, 20), sourceCanvas->getHeight() - 20);
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
			targetPointer = triangulate(sourceCanvas->getPoints(), targetCanvas->getPoints(), currentPosition);
			targetPointer->x = std::min(std::max(targetPointer->x, 20), targetCanvas->getWidth() - 20);
			targetPointer->y = std::min(std::max(targetPointer->y, 20), targetCanvas->getHeight() - 20);
			renderTarget();
		}
		else
		{
			targetPointer.reset();
		}
	}
	Refresh();
}

wxPoint ImageFrame::triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint)
{
	// move the source point in reference to the source origin
	const auto movedSource = sourcePoint - sources[0];

	// construct a basis matrix for the source triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	const auto sourceA = static_cast<float>(sources[1].x) - static_cast<float>(sources[0].x);
	const auto sourceB = static_cast<float>(sources[2].x) - static_cast<float>(sources[0].x);
	const auto sourceC = static_cast<float>(sources[1].y) - static_cast<float>(sources[0].y);
	const auto sourceD = static_cast<float>(sources[2].y) - static_cast<float>(sources[0].y);

	// construct the inverse of the source basis matrix:
	// ___1___ ( d -b )
	// ad - bc (-c  a )
	const auto sourceDeterminant = 1 / (sourceA * sourceD - sourceB * sourceC);
	const auto inverseA = sourceDeterminant * sourceD;
	const auto inverseB = sourceDeterminant * -sourceB;
	const auto inverseC = sourceDeterminant * -sourceC;
	const auto inverseD = sourceDeterminant * sourceA;

	// transform the source point into the source triangle basis
	const auto sourceU = static_cast<float>(movedSource.x) * inverseA + static_cast<float>(movedSource.y) * inverseB;
	const auto sourceV = static_cast<float>(movedSource.x) * inverseC + static_cast<float>(movedSource.y) * inverseD;

	// silently move from source triangle basis to destination triangle basis
	const auto targetU = sourceU;
	const auto targetV = sourceV;

	// construct a basis matrix for the target triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	const auto targetA = static_cast<float>(targets[1].x) - static_cast<float>(targets[0].x);
	const auto targetB = static_cast<float>(targets[2].x) - static_cast<float>(targets[0].x);
	const auto targetC = static_cast<float>(targets[1].y) - static_cast<float>(targets[0].y);
	const auto targetD = static_cast<float>(targets[2].y) - static_cast<float>(targets[0].y);

	// transform the target point from the destination triangle basis
	wxPoint target;
	target.x = static_cast<int>(std::round(targetU * targetA + targetV * targetB));
	target.y = static_cast<int>(std::round(targetU * targetC + targetV * targetD));

	// move the target point in reference to the source origin
	return target + targets[0];
}

void ImageFrame::onResize(wxSizeEvent& event)
{
	if (!IsMaximized())
	{
		const auto size = event.GetSize();
		configuration->setImageFrameSize(size.x, size.y);
		configuration->save();
	}
	event.Skip();
}

void ImageFrame::onMove(wxMoveEvent& event)
{
	if (IsMaximized())
	{
		configuration->setImageFrameMaximized(true);
	}
	else
	{
		const auto position = GetPosition();
		configuration->setImageFramePos(position.x, position.y);
		configuration->setImageFrameMaximized(false);
	}
	configuration->save();
	event.Skip();
}

void ImageFrame::onLock(const wxCommandEvent& event)
{
	if (event.GetInt() == 0)
		lock = false;
	else
		lock = true;
}

void ImageFrame::onScrollReleaseH(const wxCommandEvent& event)
{
	if (lock)
	{
		ImageCanvas* mover;
		ImageCanvas* movee;
		if (event.GetInt() == 0)
		{
			mover = sourceCanvas;
			movee = targetCanvas;
		}
		else
		{
			mover = targetCanvas;
			movee = sourceCanvas;
		}

		const auto delta = mover->GetScrollPos(wxHORIZONTAL) - mover->getOldScrollH();
		movee->Scroll(movee->GetScrollPos(wxHORIZONTAL) + delta, movee->GetScrollPos(wxVERTICAL));
		Refresh();
	}

	sourceCanvas->clearOldScrollH();
	targetCanvas->clearOldScrollH();
}

void ImageFrame::onScrollReleaseV(const wxCommandEvent& event)
{
	if (lock)
	{
		ImageCanvas* mover;
		ImageCanvas* movee;
		if (event.GetInt() == 0)
		{
			mover = sourceCanvas;
			movee = targetCanvas;
		}
		else
		{
			mover = targetCanvas;
			movee = sourceCanvas;
		}

		const auto delta = mover->GetScrollPos(wxVERTICAL) - mover->getOldScrollV();
		movee->Scroll(movee->GetScrollPos(wxHORIZONTAL), movee->GetScrollPos(wxVERTICAL) + delta);
		Refresh();
	}

	sourceCanvas->clearOldScrollV();
	targetCanvas->clearOldScrollV();
}
