#ifndef CO_POINT_H
#define CO_POINT_H

#include "Parser.h"
#include "Point.h"

class CoPoint: commonItems::parser
{
  public:
	CoPoint() = default;
	explicit CoPoint(std::istream& theStream);
	bool operator==(const CoPoint& lhs) const;

	void setName(const std::string& theName) { name = theName; }
	void setSource(const Point& thePoint) { source = thePoint; }
	void setTarget(const Point& thePoint) { target = thePoint; }

	[[nodiscard]] const auto& getName() const { return name; }
	[[nodiscard]] const auto& getSource() const { return source; }
	[[nodiscard]] const auto& getTarget() const { return target; }

	friend std::ostream& operator<<(std::ostream& output, const CoPoint& agreement);

  private:
	void registerKeys();

	std::string name;
	std::optional<Point> source;
	std::optional<Point> target;
};
std::ostream& operator<<(std::ostream& output, const CoPoint& agreement);
#endif // CO_POINT_H
