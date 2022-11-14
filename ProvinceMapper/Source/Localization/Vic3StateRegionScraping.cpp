#include "Vic3StateRegionScraping.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"
#include "StringUtils.h"

Vic3StateRegionScraping::Vic3StateRegionScraping(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void Vic3StateRegionScraping::registerKeys()
{
	registerKeyword("provinces", [this](std::istream& theStream) {
		const auto theProvinces = commonItems::stringList(theStream).getStrings();
		for (const auto& province: theProvinces)
		{
			auto theProvince = commonItems::remQuotes(province);
			std::transform(theProvince.begin(), theProvince.end(), theProvince.begin(), ::toupper);
			if (theProvince.starts_with("X"))
				theProvince = "x" + theProvince.substr(1, theProvince.length() - 1);
			if (theProvince.starts_with("0X"))
				theProvince = "x" + theProvince.substr(1, theProvince.length() - 1);
			provinces.insert(theProvince); // from "0x12345a" to x12345A
		}
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}
