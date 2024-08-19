#ifndef TRIANGULATION_POINT_PAIR_H
#define TRIANGULATION_POINT_PAIR_H
#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "LinkBase.h"

class TriangulationPointPair final: public LinkBase
{
  public:
	TriangulationPointPair() = default;
	explicit TriangulationPointPair(
		 std::istream& theStream,
		 int theID);
	explicit TriangulationPointPair(
		 int theID);

	void setSourcePoint(const wxPoint& point) { sourcePoint = point; }
	void setTargetPoint(const wxPoint& point) { targetPoint = point; }

	[[nodiscard]] const auto& getSourcePoint() const { return sourcePoint; }
	[[nodiscard]] const auto& getTargetPoint() const { return targetPoint; }
	[[nodiscard]] bool isEmpty() const { return !sourcePoint && !targetPoint && !comment; }

	[[nodiscard]] const std::string toRowString() override;
	[[nodiscard]] const wxColour getBaseRowColour() override;
	[[nodiscard]] const wxColour getActiveRowColour() override;

	friend std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);

  private:
	void registerKeys(commonItems::parser& parser);

	std::optional<wxPoint> sourcePoint;
	std::optional<wxPoint> targetPoint;
};
std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);
#endif // TRIANGULATION_POINT_PAIR_H
