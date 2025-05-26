#include "ImageFrame.h"
#include "Configuration/Configuration.h"
#include "ImageCanvas.h"
#include "LinkMapper/Triangle.h"
#include "LinkMapper/TriangulationPointPair.h"
#include "Provinces/Province.h"
#include "StatusBar.h"
#include <OSCompatibilityLayer.h>

#include <future>
#include <ranges>
#include <wx/dcbuffer.h>
#include <wx/splitter.h>
#include <wx/taskbarbutton.h>


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
	 wxFrame(parent, wxID_ANY, "Provinces", position, size, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL), configuration(std::move(theConfiguration)),
	 eventHandler(parent)
{
	taskBarBtn = MSWGetTaskBarButton();

	Bind(wxEVT_MENU, &ImageFrame::onToggleOrientation, this, wxID_REVERT);
	Bind(wxEVT_MENU, &ImageFrame::onToggleBlack, this, wxID_BOLD);
	Bind(wxEVT_MENU, &ImageFrame::onToggleTriangulationMesh, this, wxID_VIEW_SMALLICONS);
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
	Bind(wxEVT_DELAUNAY_TRIANGULATE, &ImageFrame::onDelaunayTriangulate, this);

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

	delaunayTriangulate();
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

	if (showTriangulationMesh)
	{
		renderTriangulationMesh(sourceDC, true);

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
	}

	// Draw the active triangulation pair point with a different colour.
	const auto& activeTriangulationPair = sourceCanvas->getActiveTriangulationPair();
	if (activeTriangulationPair && activeTriangulationPair->getSourcePoint())
	{
		wxPen pen = sourceDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / sourceCanvas->getScale())));
		sourceDC.SetPen(pen);
		sourceDC.SetBrush(*wxBLUE_BRUSH); // blue instead of red, to differentiate from the old 3 triangulation points per canvas
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

	if (showTriangulationMesh)
	{
		renderTriangulationMesh(targetDC, false);

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
	}

	// Draw the active triangulation pair point with a different colour.
	const auto& activeTriangulationPair = targetCanvas->getActiveTriangulationPair();
	if (activeTriangulationPair && activeTriangulationPair->getTargetPoint())
	{
		wxPen pen = targetDC.GetPen();
		pen.SetColour("white");
		pen.SetWidth(static_cast<int>(std::round(3.0 / targetCanvas->getScale())));
		targetDC.SetPen(pen);
		targetDC.SetBrush(*wxBLUE_BRUSH); // blue instead of red, to differentiate from the old 3 triangulation points per canvas
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

void ImageFrame::onToggleTriangulationMesh(wxCommandEvent& event)
{
	showTriangulationMesh = !showTriangulationMesh;
	sourceCanvas->toggleTriangulationMesh();
	targetCanvas->toggleTriangulationMesh();
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
	// Recalculate the triangulation mesh.
	delaunayTriangulate();

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

	centerMap(pt1, pt2);
}

void ImageFrame::centerMapToTriangulationPair(int pairID)
{
	for (const auto& pair: *sourceCanvas->getTriangulationPairs())
	{
		if (pair->getID() != pairID)
		{
			continue;
		}

		centerMap(pair->getSourcePoint(), pair->getTargetPoint());
		break;
	}
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
	delaunayTriangulate();

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

void ImageFrame::onDelaunayTriangulate(const wxCommandEvent& event)
{
	delaunayTriangulate();
}

void ImageFrame::centerMap(const std::optional<wxPoint>& sourceMapPoint, const std::optional<wxPoint>& targetMapPoint)
{
	const auto sourceScrollPageSizeX = sourceCanvas->GetScrollPageSize(wxHORIZONTAL);
	const auto sourceScrollPageSizeY = sourceCanvas->GetScrollPageSize(wxVERTICAL);
	const auto targetScrollPageSizeX = targetCanvas->GetScrollPageSize(wxHORIZONTAL);
	const auto targetScrollPageSizeY = targetCanvas->GetScrollPageSize(wxVERTICAL);

	wxPoint units;
	wxPoint offset;
	if (sourceMapPoint)
	{
		units = wxPoint(static_cast<int>(sourceMapPoint->x * sourceCanvas->getScale()), static_cast<int>(sourceMapPoint->y * sourceCanvas->getScale()));
		offset = wxPoint(units.x - sourceScrollPageSizeX / 2, units.y - sourceScrollPageSizeY / 2);
		sourceCanvas->Scroll(offset);
	}

	if (targetMapPoint)
	{
		units = wxPoint(static_cast<int>(targetMapPoint->x * targetCanvas->getScale()), static_cast<int>(targetMapPoint->y * targetCanvas->getScale()));
		offset = wxPoint(units.x - targetScrollPageSizeX / 2, units.y - targetScrollPageSizeY / 2);
		targetCanvas->Scroll(offset);
	}

	render();
	Refresh();
}

wxPoint findMapCornerPointEquivalent(const std::vector<std::shared_ptr<TriangulationPointPair>>& validPairs, const auto& cornerPoint)
{
	std::optional<std::shared_ptr<TriangulationPointPair>> closestPair1;
	std::optional<std::shared_ptr<TriangulationPointPair>> closestPair2;

	// Calculate the closest 2 points as 2 points with the smallest distance to the corner.
	for (const auto& pair: validPairs)
	{
		const auto& sourcePoint = pair->getSourcePoint();
		const auto& distance = std::sqrt(std::pow(cornerPoint.x - sourcePoint->x, 2) + std::pow(cornerPoint.y - sourcePoint->y, 2));

		if (!closestPair1 || distance < std::sqrt(std::pow(cornerPoint.x - (*closestPair1)->getSourcePoint()->x, 2) +
																std::pow(cornerPoint.y - (*closestPair1)->getSourcePoint()->y, 2)))
		{
			closestPair2 = closestPair1;
			closestPair1 = pair;
		}
		else if (!closestPair2 || distance < std::sqrt(std::pow(cornerPoint.x - (*closestPair2)->getSourcePoint()->x, 2) +
																	  std::pow(cornerPoint.y - (*closestPair2)->getSourcePoint()->y, 2)))
		{
			closestPair2 = pair;
		}
	}

	// Let A and B be the two closest source points to the corner.
	// Let C be the corner point.

	const auto& a = (*closestPair1)->getSourcePoint();
	const auto& b = (*closestPair2)->getSourcePoint();

	const auto& c = cornerPoint;

	// Let D and E be the equivalents of A and B on the target map.
	// Let F be the target map's equivalent of the corner point.
	// The ABC and DEF triangles should be similar.
	// We're calculating F.
	const auto& d = (*closestPair1)->getTargetPoint();
	const auto& e = (*closestPair2)->getTargetPoint();

	// 1. Calculate the scale factor between the source and target triangles.
	const double abDistance = std::sqrt(std::pow(b->x - a->x, 2) + std::pow(b->y - a->y, 2));
	const double deDistance = std::sqrt(std::pow(e->x - d->x, 2) + std::pow(e->y - d->y, 2));
	const double scale = deDistance / abDistance;

	// 2. Determine the rotation and translation.
	const double abAngle = std::atan2(b->y - a->y, b->x - a->x);
	const double deAngle = std::atan2(e->y - d->y, e->x - d->x);
	const double angle = deAngle - abAngle;

	// 3. Apply the transformation to find F.
	// 3.1. Rotate the vector AC by the angle.
	const double acX = c.x - a->x;
	const double acY = c.y - a->y;
	const double rotatedX = acX * std::cos(angle) - acY * std::sin(angle);
	const double rotatedY = acX * std::sin(angle) + acY * std::cos(angle);
	// 3.2. Scale the vector by the scale factor.
	const double scaledX = rotatedX * scale;
	const double scaledY = rotatedY * scale;
	// 3.3. Translate the scaled coordinates by D.
	const double fX = d->x + scaledX;
	const double fY = d->y + scaledY;

	const auto f = wxPoint(static_cast<int>(fX), static_cast<int>(fY));
	return f;
}

void ImageFrame::delaunayTriangulate()
{
	triangles.clear();

	// We need to have at least 3 point pairs to triangulate.
	std::vector<std::shared_ptr<TriangulationPointPair>> validPairs;

	for (auto& pair: *sourceCanvas->getTriangulationPairs())
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
	// For each source map corner, find the equivalent point on the target map.
	const auto& sourceMapWidth = sourceCanvas->getWidth();
	const auto& sourceMapHeight = sourceCanvas->getHeight();

	std::vector cornerPoints = {
		 wxPoint(0, 0),
		 wxPoint(sourceMapWidth - 1, 0),
		 wxPoint(0, sourceMapHeight - 1),
		 wxPoint(sourceMapWidth - 1, sourceMapHeight - 1),
	};

	for (const auto& cornerPoint: cornerPoints)
	{
		const auto cornerEquivalentPoint = findMapCornerPointEquivalent(validPairs, cornerPoint);

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
		cornerPair->setTargetPoint(cornerEquivalentPoint);

		validPairs.push_back(cornerPair);
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

		auto triangle = std::make_shared<Triangle>(pair1, pair2, pair3);

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
	for (const auto& triangle: triangles)
	{

		// Draw the triangle.
		if (isSourceMap)
		{
			const auto& srcPoint1 = triangle->getSourcePoint1();
			const auto& srcPoint2 = triangle->getSourcePoint2();
			const auto& srcPoint3 = triangle->getSourcePoint3();
			paintDC.DrawLine(srcPoint1.x, srcPoint1.y, srcPoint2.x, srcPoint2.y);
			paintDC.DrawLine(srcPoint2.x, srcPoint2.y, srcPoint3.x, srcPoint3.y);
			paintDC.DrawLine(srcPoint3.x, srcPoint3.y, srcPoint1.x, srcPoint1.y);
		}
		else
		{
			const auto& tgtPoint1 = triangle->getTargetPoint1();
			const auto& tgtPoint2 = triangle->getTargetPoint2();
			const auto& tgtPoint3 = triangle->getTargetPoint3();

			paintDC.DrawLine(tgtPoint1.x, tgtPoint1.y, tgtPoint2.x, tgtPoint2.y);
			paintDC.DrawLine(tgtPoint2.x, tgtPoint2.y, tgtPoint3.x, tgtPoint3.y);
			paintDC.DrawLine(tgtPoint3.x, tgtPoint3.y, tgtPoint1.x, tgtPoint1.y);
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

bool isPointInsideTriangle(const wxPoint& point, const wxPoint& vertex1, const wxPoint& vertex2, const wxPoint& vertex3)
{
	const auto sign = [](const wxPoint& p1, const wxPoint& p2, const wxPoint& p3) {
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	};

	const auto d1 = sign(point, vertex1, vertex2);
	const auto d2 = sign(point, vertex2, vertex3);
	const auto d3 = sign(point, vertex3, vertex1);

	const auto hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	const auto hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(hasNeg && hasPos);
}

void ImageFrame::autogenerateMappings()
{
	if (taskBarBtn)
	{
		taskBarBtn->SetProgressRange(20);
		taskBarBtn->SetProgressValue(0);
		taskBarBtn->SetProgressState(wxTASKBAR_BUTTON_NORMAL);
	}

	const auto& activeVersion = sourceCanvas->getActiveVersion();
	Log(LogLevel::Debug) << "Removing all existing autogenerated links...";
	std::set<int> linkIDsToRemove;
	for (const auto& link: *activeVersion->getLinks())
	{
		if (link->isAutogenerated())
		{
			linkIDsToRemove.insert(link->getID());
		}
	}
	for (const auto& linkID: linkIDsToRemove)
	{
		activeVersion->deleteLinkByID(linkID);
	}
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(1);

	// For every triangle, determine all the pixels/points inside it.
	Log(LogLevel::Debug) << "Mapping points to triangles...";
	PointToTriangleMap srcPointToTriangleMap;
	for (const auto& triangle: triangles)
	{
		const auto& sourcePoint1 = triangle->getSourcePoint1();
		const auto& sourcePoint2 = triangle->getSourcePoint2();
		const auto& sourcePoint3 = triangle->getSourcePoint3();

		// Determine the bounding box of the triangle.
		const auto minX = std::min({sourcePoint1.x, sourcePoint2.x, sourcePoint3.x});
		const auto minY = std::min({sourcePoint1.y, sourcePoint2.y, sourcePoint3.y});
		const auto maxX = std::max({sourcePoint1.x, sourcePoint2.x, sourcePoint3.x});
		const auto maxY = std::max({sourcePoint1.y, sourcePoint2.y, sourcePoint3.y});

		// For every pixel in the bounding box, determine if it's inside the triangle.
		for (auto x = minX; x <= maxX; x++)
		{
			for (auto y = minY; y <= maxY; y++)
			{
				const auto point = wxPoint(x, y);
				if (isPointInsideTriangle(point, sourcePoint1, sourcePoint2, sourcePoint3))
				{
					srcPointToTriangleMap[point] = triangle;
				}
			}
		}
	}
	Log(LogLevel::Debug) << "Determined triangles for all source map points.";
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(2);

	const auto targetMapWidth = targetCanvas->getWidth();
	const auto targetMapHeight = targetCanvas->getHeight();

	auto automapper = Automapper(activeVersion);

	const auto& tgtProvinceDefinitions = targetCanvas->getDefinitions();
	if (!tgtPointToProvinceDictInitialized)
	{
		for (const auto& tgtProvince: tgtProvinceDefinitions->getProvinces() | std::views::values)
		{
			if (tgtProvince->isWater())
			{
				for (const auto& pixel: tgtProvince->innerPixels)
					tgtPointToWaterProvinceMap[wxPoint(pixel.x, pixel.y)] = tgtProvince;
				for (const auto& pixel: tgtProvince->borderPixels)
					tgtPointToWaterProvinceMap[wxPoint(pixel.x, pixel.y)] = tgtProvince;
			}
			else
			{
				for (const auto& pixel: tgtProvince->innerPixels)
					tgtPointToLandProvinceMap[wxPoint(pixel.x, pixel.y)] = tgtProvince;
				for (const auto& pixel: tgtProvince->borderPixels)
					tgtPointToLandProvinceMap[wxPoint(pixel.x, pixel.y)] = tgtProvince;
			}
		}
		tgtPointToProvinceDictInitialized = true;
		Log(LogLevel::Debug) << "Initialized point to province dictionary for target map.";
	}


	// Exclude target provinces that are already mapped.
	// Manually mapped provinces can't be used by the automapper.
	gtl::flat_hash_set<std::string> excludedTgtProvinceIDs;
	for (const auto& tgtProvince: tgtProvinceDefinitions->getProvinces() | std::views::values)
	{
		if (activeVersion->isProvinceMapped(tgtProvince->ID, false) == Mapping::MAPPED)
			excludedTgtProvinceIDs.insert(tgtProvince->ID);
	}

	std::vector<std::shared_ptr<Province>> sourceProvinces;
	for (const auto& sourceProvince: sourceCanvas->getDefinitions()->getProvinces() | std::views::values)
		sourceProvinces.push_back(sourceProvince);

	automapper.matchTargetProvsToSourceProvs(sourceProvinces,
		 srcPointToTriangleMap,
		 tgtPointToLandProvinceMap,
		 tgtPointToWaterProvinceMap,
		 excludedTgtProvinceIDs,
		 targetMapWidth,
		 targetMapHeight);

	Log(LogLevel::Debug) << "Determined point matches for all provinces.";
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(3);

	Log(LogLevel::Debug) << "Generating links...";
	automapper.generateLinks(taskBarBtn);

	if (black == true)
	{
		// Refresh The Shade.
		sourceCanvas->generateShadedPixels();
		sourceCanvas->applyShadedPixels();
		targetCanvas->generateShadedPixels();
		targetCanvas->applyShadedPixels();
		sourceCanvas->applyStrafedPixels();
		targetCanvas->applyStrafedPixels();
	}
	render();
	Refresh();

	// Remove the progress indicator from the taskbar button.
	if (taskBarBtn)
	{
		taskBarBtn->SetProgressValue(0);
		taskBarBtn->SetProgressState(wxTASKBAR_BUTTON_NO_PROGRESS);
	}
}
