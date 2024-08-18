#pragma once
#include "TriangulationPointPair.h"

class Triangle
{
  public:
	std::shared_ptr<TriangulationPointPair> pair1;
	std::shared_ptr<TriangulationPointPair> pair2;
	std::shared_ptr<TriangulationPointPair> pair3;

   [[nodiscard]] std::vector<wxPoint> getSourcePoints() const;
	[[nodiscard]] std::vector<wxPoint> getTargetPoints() const;
};