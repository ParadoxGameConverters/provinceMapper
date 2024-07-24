#ifndef TRIANGULATION_POINT_PAIR_H
#define TRIANGULATION_POINT_PAIR_H
#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "LinkBase.h"

class TriangulationPointPair: public LinkBase
{
  public:
	TriangulationPointPair() = default;
	explicit TriangulationPointPair(
		 std::istream& theStream,
		 int theID);
	explicit TriangulationPointPair(
		 int theID);

	bool operator==(const TriangulationPointPair& rhs) const;

	void setSourcePoint(const wxPoint& point) { sourcePoint = point; }
	void setTargetPoint(const wxPoint& point) { targetPoint = point; }

	[[nodiscard]] const auto& getSourcePoint() const { return sourcePoint; }
	[[nodiscard]] const auto& getTargetPoint() const { return targetPoint; }
	[[nodiscard]] const bool isEmpty() const { return !sourcePoint && !targetPoint && !comment; }

	[[nodiscard]] const std::string toRowString();
	[[nodiscard]] const wxColour getBaseRowColour();
	[[nodiscard]] const wxColour getActiveRowColour();

	friend std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);

  private:
	void registerKeys(commonItems::parser parser);

	std::optional<wxPoint> sourcePoint;
	std::optional<wxPoint> targetPoint;
};
std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);
#endif // TRIANGULATION_POINT_PAIR_H
