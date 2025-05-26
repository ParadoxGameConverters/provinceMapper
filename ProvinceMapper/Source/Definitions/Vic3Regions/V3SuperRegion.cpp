#include "V3SuperRegion.h"
#include "V3Region.h"
#include <CommonFunctions.h>
#include <CommonRegexes.h>
#include <ParserHelpers.h>

void V3::SuperRegion::initializeSuperRegion(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void V3::SuperRegion::registerKeys()
{
	registerKeyword(commonItems::utf8BOM, [](std::istream& theStream) {
	});
	registerRegex(commonItems::catchallRegex, [this](const std::string& regionName, std::istream& theStream) {
		const auto region = std::make_shared<Region>();
		region->initializeRegion(theStream);
		regions.emplace(regionName, region);
	});
}
