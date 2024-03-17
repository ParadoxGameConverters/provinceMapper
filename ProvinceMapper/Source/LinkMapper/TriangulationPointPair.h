#ifndef TRIANGULATION_POINT_PAIR_H
#define TRIANGULATION_POINT_PAIR_H
#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Parser.h"

class TriangulationPointPair: commonItems::parser
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
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getSourcePoint() const { return sourcePoint; }
	[[nodiscard]] const auto& getTargetPoint() const { return targetPoint; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	friend std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);

  private:
	void registerKeys();

	int ID = 0;
	wxPoint sourcePoint;
	wxPoint targetPoint;
	std::optional<std::string> comment;
	std::string sourceToken;
	std::string targetToken;
};
std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);
#endif // TRIANGULATION_POINT_PAIR_H
