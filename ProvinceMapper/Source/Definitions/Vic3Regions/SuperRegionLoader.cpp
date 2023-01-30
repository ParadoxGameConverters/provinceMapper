#include "SuperRegionLoader.h"
#include "CommonFunctions.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include "V3Region.h"
#include "V3SuperRegion.h"
#include <filesystem>
#include <fstream>
#include <ranges>
namespace fs = std::filesystem;

void V3::SuperRegionLoader::loadSuperRegions(const std::string& folderPath)
{
	Log(LogLevel::Info) << "Vic3 Regions online.";
	for (const auto& fileName: commonItems::GetAllFilesInFolder(folderPath + "/../common/strategic_regions/"))
	{
		if (getExtension(fileName) != "txt")
			continue;
		std::ifstream superRegionStream(fs::u8path(folderPath + "/../common/strategic_regions/" + fileName));
		if (!superRegionStream.is_open())
			throw std::runtime_error("Could not open " + fileName + " !");

		const auto superRegion = std::make_shared<SuperRegion>();
		const auto superRegionName = trimPath(trimExtension(fileName));
		superRegion->initializeSuperRegion(superRegionStream);
		superRegions.emplace(superRegionName, superRegion);
	}

	Log(LogLevel::Info) << "Loaded " << superRegions.size() << " superregions.";
}

std::optional<std::string> V3::SuperRegionLoader::getParentRegionName(const std::string& stateName) const
{
	for (const auto& superRegion: superRegions | std::views::values)
		for (const auto& [regionName, region]: superRegion->getRegions())
			if (region->containsState(stateName))
				return regionName;
	return std::nullopt;
}

std::optional<std::string> V3::SuperRegionLoader::getParentSuperRegionName(const std::string& stateName) const
{
	for (const auto& [superRegionName, superRegion]: superRegions)
		for (const auto& region: superRegion->getRegions() | std::views::values)
			if (region->containsState(stateName))
				return superRegionName;
	return std::nullopt;
}
