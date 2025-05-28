#include "EU5Continent.h"
#include <CommonRegexes.h>
#include <ParserHelpers.h>
#include <ranges>

EU5::EU5Continent::EU5Continent(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void EU5::EU5Continent::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& superRegionName, std::istream& theStream) {
		auto superRegion = std::make_shared<EU5SuperRegion>(theStream);
		superRegions.emplace(superRegionName, superRegion);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

bool EU5::EU5Continent::continentContainsLocation(const std::string& location) const
{
	for (const auto& superRegion: superRegions | std::views::values)
		if (superRegion->superRegionContainsLocation(location))
			return true;
	return false;
}
