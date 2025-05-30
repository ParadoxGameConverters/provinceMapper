#include "EU5SuperRegion.h"
#include "CommonRegexes.h"
#include <ranges>

EU5::EU5SuperRegion::EU5SuperRegion(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void EU5::EU5SuperRegion::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& regionName, std::istream& theStream) {
		auto region = std::make_shared<EU5Region>(theStream);
		regions.emplace(regionName, region);
	});
}

bool EU5::EU5SuperRegion::superRegionContainsLocation(const std::string& location) const
{
	for (const auto& region: regions | std::views::values)
		if (region->regionContainsLocation(location))
			return true;
	return false;
}
