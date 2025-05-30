#include "EU5Area.h"
#include <CommonRegexes.h>
#include <ParserHelpers.h>
#include <ranges>

EU5::EU5Area::EU5Area(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void EU5::EU5Area::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& provinceName, std::istream& theStream) {
		const auto locationsList = commonItems::getStrings(theStream);
		auto province = std::make_shared<EU5Province>(locationsList);
		provinces.emplace(provinceName, province);
	});
}

bool EU5::EU5Area::areaContainsLocation(const std::string& location) const
{
	for (const auto& province: provinces | std::views::values)
		if (province->provinceContainsLocation(location))
			return true;
	return false;
}
