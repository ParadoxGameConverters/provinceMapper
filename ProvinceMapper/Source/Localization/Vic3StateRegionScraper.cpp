#include "Vic3StateRegionScraper.h"
#include "Vic3StateRegionScraping.h"
#include <CommonRegexes.h>
#include <ParserHelpers.h>

#include "Log.h"

Vic3StateRegionScraper::Vic3StateRegionScraper(const std::filesystem::path& fileName)
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
			if (scraping.isProvincePort(province))
				localizations.emplace(province + "_port", "HUB_NAME_" + stateName + "_port");
			if (scraping.isProvinceFarm(province))
				localizations.emplace(province + "_farm", "HUB_NAME_" + stateName + "_farm");
			if (scraping.isProvinceMine(province))
				localizations.emplace(province + "_mine", "HUB_NAME_" + stateName + "_mine");
			if (scraping.isProvinceWood(province))
				localizations.emplace(province + "_wood", "HUB_NAME_" + stateName + "_wood");

			localizations.emplace(province, stateName);
		}
	});
}
