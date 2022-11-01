#include "Vic3StateRegionScraper.h"
#include "ParserHelpers.h"
#include "Vic3StateRegionScraping.h"
#include <CommonRegexes.h>

#include "Log.h"

Vic3StateRegionScraper::Vic3StateRegionScraper(const std::string& fileName)
{
	registerKeys();
	parseFile(fileName);
	clearRegisteredKeywords();
}

void Vic3StateRegionScraper::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& stateName, std::istream& theStream) {
		const auto theProvinces = Vic3StateRegionScraping(theStream).getProvinces();
		for (const auto& province: theProvinces)
		{
			Log(LogLevel::Debug) << "emplacing: " << province << " - " << stateName;
			localizations.emplace(province, stateName);
		}
	});
}
