#include "LocalizationMapper.h"
#include "CsvScraper.h"
#include "OSCompatibilityLayer.h"
#include "Vic3StateRegionScraper.h"
#include "YmlScraper.h"

void LocalizationMapper::scrapeSourceDir(const std::string& dirPath)
{
	std::string actualPath;
	// our dirpath is a path to the map folder. locs are elsewhere.
	if (commonItems::DoesFolderExist(dirPath + "/state_regions")) // vic3
		actualPath = dirPath + "/state_regions";
	else if (commonItems::DoesFolderExist(dirPath + "/../localisation")) // ck2, eu4, vic2
		actualPath = dirPath + "/../localisation";
	else if (commonItems::DoesFolderExist(dirPath + "/../localization/english")) // ck3, imp
		actualPath = dirPath + "/../localization/english";
	else if (actualPath.empty())
		return;

	for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
		scrapeFile(actualPath + "/" + fileName, LocType::SOURCE);
}

void LocalizationMapper::scrapeTargetDir(const std::string& dirPath)
{
	std::string actualPath;
	if (commonItems::DoesFolderExist(dirPath + "/state_regions")) // vic3
		actualPath = dirPath + "/state_regions";
	else if (commonItems::DoesFolderExist(dirPath + "/../localisation")) // ck2, eu4, vic2
		actualPath = dirPath + "/../localisation";
	else if (commonItems::DoesFolderExist(dirPath + "/../localization/english")) // ck3, imp
		actualPath = dirPath + "/../localization/english";
	else if (actualPath.empty())
		return;

	for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
		scrapeFile(actualPath + "/" + fileName, LocType::TARGET);
}

void LocalizationMapper::scrapeFile(const std::string& filePath, LocType locType)
{
	if (filePath.find(".csv") != std::string::npos)
	{
		const auto locs = CsvScraper(filePath).getLocalizations();
		if (locType == LocType::SOURCE)
			sourceLocalizations.insert(locs.begin(), locs.end());
		else
			targetLocalizations.insert(locs.begin(), locs.end());
	}
	else if (filePath.find(".yml") != std::string::npos)
	{
		const auto locs = YmlScraper(filePath).getLocalizations();
		if (locType == LocType::SOURCE)
			sourceLocalizations.insert(locs.begin(), locs.end());
		else
			targetLocalizations.insert(locs.begin(), locs.end());
	}
	else if (filePath.find(".txt") != std::string::npos)
	{
		const auto locs = Vic3StateRegionScraper(filePath).getLocalizations();
		if (locType == LocType::SOURCE)
			sourceLocalizations.insert(locs.begin(), locs.end());
		else
			targetLocalizations.insert(locs.begin(), locs.end());
	}
}

std::optional<std::string> LocalizationMapper::getLocForSourceKey(const std::string& key) const
{
	if (const auto& keyItr = sourceLocalizations.find(key); keyItr != sourceLocalizations.end())
		return keyItr->second;
	else
		return std::nullopt;
}

std::optional<std::string> LocalizationMapper::getLocForTargetKey(const std::string& key) const
{
	if (const auto& keyItr = targetLocalizations.find(key); keyItr != targetLocalizations.end())
		return keyItr->second;
	else
		return std::nullopt;
}