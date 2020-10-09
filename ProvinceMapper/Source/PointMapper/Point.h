#ifndef POINT_H
#define POINT_H

#include <ostream>

struct Point
{
	Point() = default;
	explicit Point(int xCoord, int yCoord);
	bool operator==(const Point& lhs) const;
	bool operator!=(const Point& lhs) const;
	bool operator!() const;

	int x = 0;
	int y = 0;
};

std::ostream& operator<<(std::ostream& output, const Point& point);

#endif // CO_POINT_H
