#include "EU5Region.h"
#include <CommonRegexes.h>
#include <ParserHelpers.h>
#include <ranges>

EU5::EU5Region::EU5Region(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void EU5::EU5Region::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& areaName, std::istream& theStream) {
		auto area = std::make_shared<EU5Area>(theStream);
		areas.emplace(areaName, area);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

bool EU5::EU5Region::regionContainsLocation(const std::string& location) const
{
	for (const auto& area: areas | std::views::values)
		if (area->areaContainsLocation(location))
			return true;
	return false;
}
