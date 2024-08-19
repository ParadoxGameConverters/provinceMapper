#include "Triangle.h"

std::vector<wxPoint> Triangle::getSourcePoints() const
{
	return std::vector{*pair1->getSourcePoint(), *pair2->getSourcePoint(), *pair3->getSourcePoint()};
}

std::vector<wxPoint> Triangle::getTargetPoints() const
{
	return std::vector{*pair1->getTargetPoint(), *pair2->getTargetPoint(), *pair3->getTargetPoint()};
}
