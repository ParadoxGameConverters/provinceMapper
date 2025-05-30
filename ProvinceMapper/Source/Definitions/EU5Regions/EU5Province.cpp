#include "EU5Province.h"

EU5::EU5Province::EU5Province(const std::vector<std::string>& theLocations)
{
	for (const auto& location: theLocations)
		locations.emplace(location);
}

bool EU5::EU5Province::provinceContainsLocation(const std::string& location) const
{
	return locations.contains(location);
}
