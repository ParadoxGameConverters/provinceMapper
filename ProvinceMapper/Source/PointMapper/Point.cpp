#include "Point.h"
#include <string>

Point::Point(int xCoord, int yCoord): x(xCoord), y(yCoord){}

bool Point::operator==(const Point& lhs) const
{
	return x == lhs.x && y == lhs.y;
}

bool Point::operator!=(const Point& lhs) const
{
	return x != lhs.x || y != lhs.y;
}

bool Point::operator!() const
{
	return !x && !y;
}

std::ostream& operator<<(std::ostream& output, const Point& point)
{
	output << std::to_string(point.x) << "x" << std::to_string(point.y);
	return output;
}