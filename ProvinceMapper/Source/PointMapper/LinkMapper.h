#ifndef POINT_MAPPER_H
#define POINT_MAPPER_H

#include "CoPoint.h"
#include "Parser.h"

class PointMapper: commonItems::parser
{
  public:
	PointMapper();
	explicit PointMapper(std::istream& theStream);

	void exportPoints() const;
	void exportAdjustedMap() const;
	[[nodiscard]] const auto& getCoPoints() const { return coPoints; }

  private:
	void registerKeys();
	std::shared_ptr<std::vector<std::shared_ptr<CoPoint>>> coPoints;
};

#endif // POINT_MAPPER_H