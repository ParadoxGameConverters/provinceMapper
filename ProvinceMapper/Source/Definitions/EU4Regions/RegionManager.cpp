#include "RegionManager.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include <filesystem>
#include <fstream>
#include <ranges>
namespace fs = std::filesystem;

void EU4::RegionManager::loadRegions(const std::string& EU4Path)
{
	Log(LogLevel::Info) << "EU4 Region manager online.";

	auto areaFilename = EU4Path + "/area.txt";
	auto regionFilename = EU4Path + "/region.txt";
	auto superRegionFilename = EU4Path + "/superregion.txt";

	std::ifstream areaStream(fs::u8path(areaFilename));
	if (!areaStream.is_open())
		throw std::runtime_error("Could not open area.txt!");
	registerAreaKeys();
	parseStream(areaStream);
	clearRegisteredKeywords();
	areaStream.close();

	std::ifstream superRegionStream(fs::u8path(superRegionFilename));
	if (!superRegionStream.is_open())
		throw std::runtime_error("Could not open superregion.txt!");
	registerSuperRegionKeys();
	parseStream(superRegionStream);
	clearRegisteredKeywords();
	superRegionStream.close();

	std::ifstream regionStream(fs::u8path(regionFilename));
	if (!regionStream.is_open())
		throw std::runtime_error("Could not open region.txt!");
	registerRegionKeys();
	parseStream(regionStream);
	clearRegisteredKeywords();
	regionStream.close();

	linkSuperRegions();
	linkRegions();

	Log(LogLevel::Info) << "EU4 Region manager : " << areas.size() << " areas, " << regions.size() << " regions, " << superRegions.size() << " superregions.";
}

void EU4::RegionManager::registerAreaKeys()
{
	registerRegex(R"([\w_]+)", [this](const std::string& areaName, std::istream& theStream) {
		areas.emplace(areaName, std::make_shared<Area>(theStream));
	});
}

void EU4::RegionManager::registerSuperRegionKeys()
{
	registerRegex(R"([\w_]+)", [this](const std::string& superRegionName, std::istream& theStream) {
		const commonItems::stringList theList(theStream);
		std::vector<std::string> tempRegions;
		for (const auto& region: theList.getStrings())
		{
			if (region == "restrict_charter")
				continue;
			tempRegions.emplace_back(region);
		}
		superRegions.emplace(superRegionName, std::make_shared<SuperRegion>(tempRegions));
	});
}

void EU4::RegionManager::registerRegionKeys()
{
	registerRegex(R"([\w_]+)", [this](const std::string& regionName, std::istream& theStream) {
		regions.emplace(regionName, std::make_shared<Region>(theStream));
	});
}

std::optional<std::string> EU4::RegionManager::getParentAreaName(const std::string& provinceID) const
{
	for (const auto& [areaName, area]: areas)
		if (area->areaContainsProvince(provinceID))
			return areaName;

	return std::nullopt;
}

std::optional<std::string> EU4::RegionManager::getParentRegionName(const std::string& provinceID) const
{
	for (const auto& [regionName, region]: regions)
		if (region->regionContainsProvince(provinceID))
			return regionName;

	return std::nullopt;
}

std::optional<std::string> EU4::RegionManager::getParentSuperRegionName(const std::string& provinceID) const
{
	for (const auto& [superRegionName, superRegion]: superRegions)
		if (superRegion->superRegionContainsProvince(provinceID))
			return superRegionName;

	return std::nullopt;
}

void EU4::RegionManager::linkSuperRegions()
{
	for (const auto& [superRegionName, superRegion]: superRegions)
	{
		const auto& requiredRegions = superRegion->getRegions();
		for (const auto& requiredRegionName: requiredRegions | std::views::keys)
		{
			const auto& regionItr = regions.find(requiredRegionName);
			if (regionItr != regions.end())
			{
				superRegion->linkRegion(std::pair(regionItr->first, regionItr->second));
			}
			else
			{
				throw std::runtime_error("Superregion's " + superRegionName + " region " + requiredRegionName + " does not exist!");
			}
		}
	}
}

void EU4::RegionManager::linkRegions()
{
	for (const auto& [regionName, region]: regions)
	{
		const auto& requiredAreas = region->getAreas();
		for (const auto& requiredAreaName: requiredAreas | std::views::keys)
		{
			const auto& areaItr = areas.find(requiredAreaName);
			if (areaItr != areas.end())
			{
				region->linkArea(std::pair(areaItr->first, areaItr->second));
			}
			else
			{
				throw std::runtime_error("Region's " + regionName + " area " + requiredAreaName + " does not exist!");
			}
		}
	}
}
