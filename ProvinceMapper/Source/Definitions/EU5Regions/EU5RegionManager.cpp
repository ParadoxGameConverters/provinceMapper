#include "EU5RegionManager.h"
#include "CommonRegexes.h"
#include <Log.h>
#include <OSCompatibilityLayer.h>
#include <ParserHelpers.h>
#include <filesystem>
#include <fstream>
#include <ranges>
namespace fs = std::filesystem;

void EU5::EU5RegionManager::loadContinents(const std::filesystem::path& EU5Path)
{
	Log(LogLevel::Info) << "EU5 Region manager online.";

	auto definitionFile = EU5Path / "definitions.txt";

	registerKeys();
	parseFile(definitionFile);
	clearRegisteredKeywords();

	Log(LogLevel::Info) << "EU5 Region manager : " << continents.size() << " continents.";
}

void EU5::EU5RegionManager::registerKeys()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& continentName, std::istream& theStream) {
		auto continent = std::make_shared<EU5Continent>(theStream);
		continents.emplace(continentName, continent);
	});
}

std::optional<std::string> EU5::EU5RegionManager::getParentProvinceName(const std::string& location) const
{
	for (const auto& continent: continents | std::views::values)
		if (continent->continentContainsLocation(location))
		{
			for (const auto& superRegion: continent->getSuperRegions() | std::views::values)
				if (superRegion->superRegionContainsLocation(location))
				{
					for (const auto& region: superRegion->getRegions() | std::views::values)
						if (region->regionContainsLocation(location))
						{
							for (const auto& area: region->getAreas() | std::views::values)
								if (area->areaContainsLocation(location))
								{
									for (const auto& [provinceName, province]: area->getProvinces())
										if (province->provinceContainsLocation(location))
										{
											return provinceName;
										}
								}
						}
				}
		}

	return std::nullopt;
}

std::optional<std::string> EU5::EU5RegionManager::getParentAreaName(const std::string& location) const
{
	for (const auto& continent: continents | std::views::values)
		if (continent->continentContainsLocation(location))
		{
			for (const auto& superRegion: continent->getSuperRegions() | std::views::values)
				if (superRegion->superRegionContainsLocation(location))
				{
					for (const auto& region: superRegion->getRegions() | std::views::values)
						if (region->regionContainsLocation(location))
						{
							for (const auto& [areaName, area]: region->getAreas())
								if (area->areaContainsLocation(location))
								{
									return areaName;
								}
						}
				}
		}

	return std::nullopt;
}

std::optional<std::string> EU5::EU5RegionManager::getParentRegionName(const std::string& location) const
{
	for (const auto& continent: continents | std::views::values)
		if (continent->continentContainsLocation(location))
		{
			for (const auto& superRegion: continent->getSuperRegions() | std::views::values)
				if (superRegion->superRegionContainsLocation(location))
				{
					for (const auto& [regionName, region]: superRegion->getRegions())
						if (region->regionContainsLocation(location))
						{
							return regionName;
						}
				}
		}

	return std::nullopt;
}

std::optional<std::string> EU5::EU5RegionManager::getParentSuperRegionName(const std::string& location) const
{
	for (const auto& continent: continents | std::views::values)
		if (continent->continentContainsLocation(location))
		{
			for (const auto& [superRegionName, superRegion]: continent->getSuperRegions())
				if (superRegion->superRegionContainsLocation(location))
				{
					return superRegionName;
				}
		}

	return std::nullopt;
}

std::optional<std::string> EU5::EU5RegionManager::getParentContinentName(const std::string& location) const
{
	for (const auto& [continentName, continent]: continents)
		if (continent->continentContainsLocation(location))
		{
			return continentName;
		}

	return std::nullopt;
}