#ifndef EU5_CONTINENT_H
#define EU5_CONTINENT_H
#include "EU5SuperRegion.h"
#include <Parser.h>

namespace EU5
{
class EU5Continent: commonItems::parser
{
  public:
	EU5Continent() = default;
	explicit EU5Continent(std::istream& theStream);

	[[nodiscard]] const auto& getSuperRegions() const { return superRegions; }
	[[nodiscard]] bool continentContainsLocation(const std::string& location) const;

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<EU5SuperRegion>> superRegions;
};
} // namespace EU5

#endif // EU5_CONTINENT_H