#include "ImageBox.h"
#include "PointWindow.h"
#include "Log.h"
#include "wx/image.h"
#include "../PointMapper/CoPoint.h"

wxDEFINE_EVENT(wxEVT_POINT_PLACED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DESELECT_POINT, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_DELETE_WORKING_POINT, wxCommandEvent);

ImageBox::ImageBox(wxWindow* parent, const wxImage& theImage, ImageTabSelector theSelector): wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	// ImageBox is the raw image + display functions.
	
	eventListener = parent; // command events will propagate up the thread to someone who's listening, likely mainframe.
	selector = theSelector; // We should know which of the images we are.
	
	Bind(wxEVT_PAINT, &ImageBox::paintEvent, this);
	Bind(wxEVT_SIZE, &ImageBox::onSize, this);
	Bind(wxEVT_RIGHT_UP, &ImageBox::rightUp, this);
	Bind(wxEVT_LEFT_UP, &ImageBox::leftUp, this);
	Bind(wxEVT_KEY_DOWN, &ImageBox::onKeyDown, this);

	image = theImage;
	dc = new wxClientDC(this); // Device Context, a drawable area.
	bitmap = wxBitmap(image);
	SetMinSize(bitmap.GetSize());
}

void ImageBox::paintEvent(wxPaintEvent& evt)
{
	render();
}
void ImageBox::paintNow()
{
	for (const auto& point: knownPoints)
	{
		auto red = image.GetRed(point.x, point.y);
		auto green = image.GetGreen(point.x, point.y);
		auto blue = image.GetBlue(point.x, point.y);		
		auto tempimage = image;
		//tempimage.SetMask();
		tempimage.SetMaskFromImage(tempimage, red, green, blue);
		//auto tempbitmap = wxBitmap(tempimage);
		//auto mask = new wxMask(tempbitmap, "black");
		//auto maskbitmap = mask->GetBitmap();
		//tempimage.
		bitmap = wxBitmap(tempimage);
		
	}
		
	// Log(LogLevel::Debug) << "mask " << std::to_string(red) << " " << std::to_string(green) << " " << std::to_string(blue);

	// bitmap.SetMask(mask);

	
	render();
}
void ImageBox::render()
{
	dc->DrawBitmap(bitmap, 0, 0, false);
}
void ImageBox::onSize(wxSizeEvent& event)
{
	paintNow();
	event.Skip();
}

void ImageBox::rightUp(wxMouseEvent& event)
{
	// Transliterate non-propagating mouse events to a propagating command event.
	const Point point(event.GetPosition().x, event.GetPosition().y);
	wxCommandEvent evt(wxEVT_POINT_PLACED);
	evt.SetClientData(new PointData(point, selector));
	
	// let it loose up the thread.
	eventListener->QueueEvent(evt.Clone());

	// We draw nothing until told to by someone smarter.
}

void ImageBox::registerPoint(const Point& point)
{
	knownPoints.emplace_back(point);
}

void ImageBox::updatePoint(wxCommandEvent& event)
{
	auto const* pointData = static_cast<PointData*>(event.GetClientData());
	if (pointData->getSelector() == selector)
	{
		// It's for us! Hallo? Hallo?
		auto updatedPoint = pointData->getPoint();
		if (pointData->getReplacementPoint())
		{
			for (auto& point: knownPoints)
				if (point == updatedPoint)
				{
					point = *pointData->getReplacementPoint();
					break;
				}
		}
		else if (pointData->getDrop())
		{
			auto point = knownPoints.begin();
			while (point != knownPoints.end())
			{
				if (*point == pointData->getPoint())
				{
					knownPoints.erase(point);
					break;
				}
				++point;
			}
		}
		else
		{
			registerPoint(updatedPoint);
		}
	}
}

void ImageBox::leftUp(wxMouseEvent& event)
{
	// Left up means deselect working point if any.
	wxCommandEvent evt(wxEVT_DESELECT_POINT);
	eventListener->QueueEvent(evt.Clone());
}

void ImageBox::onKeyDown(wxKeyEvent& event)
{
	wxCommandEvent evt(wxEVT_CHANGE_TAB);
	switch (event.GetKeyCode())
	{
		case WXK_F1:
			evt.SetInt(1);
			eventListener->QueueEvent(evt.Clone());
			break;
		case WXK_F2:
			evt.SetInt(2);
			eventListener->QueueEvent(evt.Clone());
			break;
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
			evt.SetEventType(wxEVT_DELETE_WORKING_POINT);
			eventListener->QueueEvent(evt.Clone());
		default:
			event.Skip();
	}
}
