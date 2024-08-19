#pragma once
#include "TriangulationPointPair.h"

class Triangle
{
  public:
	Triangle(std::shared_ptr<TriangulationPointPair> pair1, std::shared_ptr<TriangulationPointPair> pair2, std::shared_ptr<TriangulationPointPair> pair3):
		 pair1(std::move(pair1)), pair2(std::move(pair2)), pair3(std::move(pair3))
	{
	}

	[[nodiscard]] auto getSourcePoint1() const { return *pair1->getSourcePoint(); }
	[[nodiscard]] auto getSourcePoint2() const { return *pair2->getSourcePoint(); }
	[[nodiscard]] auto getSourcePoint3() const { return *pair3->getSourcePoint(); }
	[[nodiscard]] auto getTargetPoint1() const { return *pair1->getTargetPoint(); }
	[[nodiscard]] auto getTargetPoint2() const { return *pair2->getTargetPoint(); }
	[[nodiscard]] auto getTargetPoint3() const { return *pair3->getTargetPoint(); }

   [[nodiscard]] std::vector<wxPoint> getSourcePoints() const;
	[[nodiscard]] std::vector<wxPoint> getTargetPoints() const;

  private:
	std::shared_ptr<TriangulationPointPair> pair1;
	std::shared_ptr<TriangulationPointPair> pair2;
	std::shared_ptr<TriangulationPointPair> pair3;
};
