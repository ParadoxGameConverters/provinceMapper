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
		const auto scraping = Vic3StateRegionScraping(theStream);
		const auto& theProvinces = scraping.getProvinces();
		for (const auto& province: theProvinces)
		{
			if (scraping.isProvinceCity(province))
				localizations.emplace(province + "_city", "HUB_NAME_" + stateName + "_city");
			else if (scraping.isProvincePort(province))
				localizations.emplace(province + "_port", "HUB_NAME_" + stateName + "_port");
			else if (scraping.isProvinceFarm(province))
				localizations.emplace(province + "_farm", "HUB_NAME_" + stateName + "_farm");
			else if (scraping.isProvinceMine(province))
				localizations.emplace(province + "_mine", "HUB_NAME_" + stateName + "_mine");
			else if (scraping.isProvinceWood(province))
				localizations.emplace(province + "_wood", "HUB_NAME_" + stateName + "_wood");

			localizations.emplace(province, stateName);
		}
	});
}
