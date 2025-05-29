#include "Vic3StateRegionScraping.h"
#include <CommonRegexes.h>
#include <ParserHelpers.h>
#include <StringUtils.h>

namespace
{
std::string prepProvinceName(const std::string& theName)
{
	auto theProvince = theName;
	std::transform(theProvince.begin(), theProvince.end(), theProvince.begin(), ::toupper);
	if (theProvince.starts_with("X"))
		theProvince = "x" + theProvince.substr(1, theProvince.length() - 1);
	if (theProvince.starts_with("0X"))
		theProvince = "x" + theProvince.substr(2, theProvince.length() - 2);
	return theProvince;
}
} // namespace

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
			provinces.insert(prepProvinceName(province)); // from "0x12345a" to x12345A
	});
	registerKeyword("city", [this](std::istream& theStream) {
		cities.emplace(prepProvinceName(commonItems::getString(theStream)));
	});
	registerKeyword("port", [this](std::istream& theStream) {
		ports.emplace(prepProvinceName(commonItems::getString(theStream)));
	});
	registerKeyword("farm", [this](std::istream& theStream) {
		farms.emplace(prepProvinceName(commonItems::getString(theStream)));
	});
	registerKeyword("mine", [this](std::istream& theStream) {
		mines.emplace(prepProvinceName(commonItems::getString(theStream)));
	});
	registerKeyword("wood", [this](std::istream& theStream) {
		woods.emplace(prepProvinceName(commonItems::getString(theStream)));
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}
