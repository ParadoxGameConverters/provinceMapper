#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../PointMapper/CoPoint.h"

enum class ImageTabSelector
{
	SOURCE,
	TARGET
};

// This is a small wrapper for our command event payloads that carries click coordinates and similar point data.
class PointData: public wxClientData
{
  public:
	PointData(Point thePoint, ImageTabSelector theSelector): point(thePoint), selector(theSelector) {}
	PointData(Point thePoint, Point replacement, ImageTabSelector theSelector): point(thePoint), replacementPoint(replacement), selector(theSelector) {}
	[[nodiscard]] const auto& getPoint() const { return point; }
	[[nodiscard]] const auto& getReplacementPoint() const { return replacementPoint; }
	[[nodiscard]] auto getSelector() const { return selector; }
	[[nodiscard]] auto getDrop() const { return drop; }
	void setDrop() { drop = true; }

  private:
	Point point;
	std::optional<Point> replacementPoint;
	ImageTabSelector selector;
	bool drop = false;
};
