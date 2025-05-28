#include "EU5Definitions.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <CommonRegexes.h>
#include <OSCompatibilityLayer.h>
#include <ParserHelpers.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

namespace
{
std::tuple<int, int, int> breakDownChroma(const std::string& theChroma)
{
	const auto r = std::stoi(theChroma.substr(0, 2), nullptr, 16);
	const auto g = std::stoi(theChroma.substr(2, 2), nullptr, 16);
	const auto b = std::stoi(theChroma.substr(4, 2), nullptr, 16);

	return {r, g, b};
}

std::string tolower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return str;
}

} // namespace

void EU5Definitions::loadDefinitions(const fs::path& mapDataPath, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	if (!commonItems::DoesFileExist(mapDataPath / "named_locations/00_default.txt"))
		throw std::runtime_error("Named Locations file cannot be found!");

	if (commonItems::DoesFileExist(mapDataPath / "definitions.txt"))
		eu5RegionManager.loadContinents(mapDataPath);

	// A bit of misnomer. Definitions file is actually the named_locations/00_default.txt one.
	// definitions.txt is actually hierarchy file, akin to eu4's area.txt, regions.txt etc.

	registerKeys();
	defParser.parseFile(mapDataPath / "named_locations/00_default.txt");
	defParser.clearRegisteredKeywords();

	filloutProvinceDetails(localizationMapper, locType);

	tryToLoadProvinceTypes(mapDataPath);
}

void EU5Definitions::registerKeys()
{
	defParser.registerRegex(commonItems::catchallRegex, [this](const std::string& locationName, std::istream& theStream) {
		auto chroma = commonItems::getString(theStream);

		// First deal with shit.
		if (chroma.size() > 6)
		{
			chroma = chroma.substr(0, 6);
		}

		if (chroma.size() != 6)
		{
			chroma.insert(0, 6 - chroma.size(), '0');
		}

		// Chroma needs to be broken into rgb ints.
		const auto [r, g, b] = breakDownChroma(chroma);
		auto province = std::make_shared<Province>(locationName, r, g, b, locationName); // id is the same as the name.
		provinces.emplace(locationName, province);
	});
}

void EU5Definitions::filloutProvinceDetails(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	for (const auto& [provinceName, province]: provinces)
	{
		// locs
		if (locType == LocalizationMapper::LocType::SOURCE)
		{
			auto locName = localizationMapper.getLocForSourceKey(provinceName);
			if (locName && !locName->empty())
			{
				Log(LogLevel::Debug) << "for sourcename for " << provinceName << " : " << *locName;
				province->locName = *locName;
			}
		}
		else
		{
			auto locName = localizationMapper.getLocForTargetKey(provinceName);
			if (locName && !locName->empty())
				province->locName = *locName;
		}

		// regionals
		if (locType == LocalizationMapper::LocType::SOURCE)
		{
			if (const auto& regName = eu5RegionManager.getParentProvinceName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForSourceKey(*regName);
				if (locName && !locName->empty())
					province->setProvinceName(*locName);
				else
					province->setProvinceName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentAreaName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForSourceKey(*regName);
				if (locName && !locName->empty())
					province->setAreaName(*locName);
				else
					province->setAreaName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentRegionName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForSourceKey(*regName);
				if (locName && !locName->empty())
					province->setRegionName(*locName);
				else
					province->setRegionName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentSuperRegionName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForSourceKey(*regName);
				if (locName && !locName->empty())
					province->setSuperRegionName(*locName);
				else
					province->setSuperRegionName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentContinentName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForSourceKey(*regName);
				if (locName && !locName->empty())
					province->setContinentName(*locName);
				else
					province->setContinentName(*regName);
			}
		}
		else
		{
			if (const auto& regName = eu5RegionManager.getParentProvinceName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForTargetKey(*regName);
				if (locName && !locName->empty())
					province->setProvinceName(*locName);
				else
					province->setProvinceName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentAreaName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForTargetKey(*regName);
				if (locName && !locName->empty())
					province->setAreaName(*locName);
				else
					province->setAreaName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentRegionName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForTargetKey(*regName);
				if (locName && !locName->empty())
					province->setRegionName(*locName);
				else
					province->setRegionName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentSuperRegionName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForTargetKey(*regName);
				if (locName && !locName->empty())
					province->setSuperRegionName(*locName);
				else
					province->setSuperRegionName(*regName);
			}
			if (const auto& regName = eu5RegionManager.getParentContinentName(provinceName); regName)
			{
				const auto& locName = localizationMapper.getLocForTargetKey(*regName);
				if (locName && !locName->empty())
					province->setContinentName(*locName);
				else
					province->setContinentName(*regName);
			}
		}

		// chromaCache
		chromaCache.insert(std::pair(pixelPack(province->r, province->g, province->b), province));
	}
}

void EU5Definitions::registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chromaItr = chromaCache.find(pixelPack(r, g, b));
	if (chromaItr != chromaCache.end())
		chromaItr->second->innerPixels.emplace_back(pixel);
}

void EU5Definitions::registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chromaItr = chromaCache.find(pixelPack(r, g, b));
	if (chromaItr != chromaCache.end())
		chromaItr->second->borderPixels.emplace_back(pixel);
}

std::optional<std::string> EU5Definitions::getNameForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->bespokeName();
	return std::nullopt;
}

std::optional<std::string> EU5Definitions::getMiscForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->miscName();
	return std::nullopt;
}

std::optional<std::string> EU5Definitions::getIDForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->ID;
	return std::nullopt;
}

std::shared_ptr<Province> EU5Definitions::getProvinceForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second;
	return nullptr;
}

std::shared_ptr<Province> EU5Definitions::getProvinceForID(const std::string& ID)
{
	if (const auto& provinceItr = provinces.find(ID); provinceItr != provinces.end())
		if (provinceItr->second)
			return provinceItr->second;
	return nullptr;
}

void EU5Definitions::loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	for (const auto& [id, province]: provinces)
	{
		if (locType == LocalizationMapper::LocType::SOURCE && localizationMapper.getLocForSourceKey(id))
			province->locName = *localizationMapper.getLocForSourceKey(id);
		if (locType == LocalizationMapper::LocType::TARGET && localizationMapper.getLocForTargetKey(id))
			province->locName = *localizationMapper.getLocForTargetKey(id);
	}
}

void EU5Definitions::registerNeighbor(unsigned int provinceChroma, unsigned int neighborChroma)
{
	if (!neighborChromas.contains(provinceChroma))
		neighborChromas.emplace(provinceChroma, std::set<unsigned int>{});
	neighborChromas.at(provinceChroma).emplace(neighborChroma);
}

std::map<unsigned int, std::set<unsigned int>> EU5Definitions::getNeighborChromas() const
{
	return neighborChromas;
}

void EU5Definitions::ditchAdjacencies(const fs::path& fileName)
{
	std::map<std::string, std::set<std::string>> adjacencies;
	for (const auto& [sourceChroma, targetChromas]: neighborChromas)
	{
		if (const auto& sourceProvince = getIDForChroma(sourceChroma); sourceProvince)
		{
			adjacencies.emplace(*sourceProvince, std::set<std::string>{});
			for (const auto& targetChroma: targetChromas)
			{
				if (const auto& targetProvince = getIDForChroma(targetChroma); targetProvince)
					adjacencies.at(*sourceProvince).emplace(*targetProvince);
			}
		}
	}
	std::ofstream adjacenciesFile(fileName);
	for (const auto& [sourceProvince, targetProvinces]: adjacencies)
	{
		if (targetProvinces.empty())
			continue;
		adjacenciesFile << sourceProvince << " = { ";
		for (const auto& targetProvince: targetProvinces)
			adjacenciesFile << targetProvince << " ";
		adjacenciesFile << "}\n";
	}
	adjacenciesFile.close();
}

void EU5Definitions::tryToLoadProvinceTypes(const fs::path& mapDataPath)
{
	const fs::path filePath = mapDataPath / "default.map";
	if (!commonItems::DoesFileExist(filePath))
	{
		return;
	}

	auto parser = commonItems::parser();
	const std::string provinceTypesRegex =
		 "sea_zones|wasteland|impassable_terrain|uninhabitable|river_provinces|lakes|LAKES|impassable_mountains|impassable_seas|non_ownable";

	parser.registerRegex(provinceTypesRegex, [&](const std::string& provinceType, std::istream& stream) {
		std::string lowerCaseProvinceType = tolower(provinceType);

		auto provIds = commonItems::getStrings(stream);
		for (auto& id: provIds)
		{
			if (provinces.contains(id) && provinces.at(id))
				provinces[id]->addProvinceType(lowerCaseProvinceType);
		}
	});

	parser.IgnoreUnregisteredItems();
	parser.parseFile(filePath);
}
