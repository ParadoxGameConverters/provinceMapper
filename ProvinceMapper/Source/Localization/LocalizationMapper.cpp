#include "LocalizationMapper.h"
#include "CsvScraper.h"
#include "Vic3StateRegionScraper.h"
#include "YmlScraper.h"
#include <OSCompatibilityLayer.h>

void LocalizationMapper::scrapeSourceDir(const std::filesystem::path& dirPath)
{
	std::filesystem::path actualPath;
	// our dirpath is a path to the map folder. locs are elsewhere.
	if (commonItems::DoesFolderExist(dirPath / "state_regions")) // vic3 scraping override
	{
		actualPath = dirPath / "state_regions";
		for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
			scrapeFile(actualPath / fileName, LocType::SOURCE);
		actualPath.clear();
	}

	if (commonItems::DoesFolderExist(dirPath / "../localisation")) // ck2, eu4, vic2
	{
		actualPath = dirPath / "../localisation";
		Log(LogLevel::Info) << "Priming for Ck2/EU4/Vic2 Source Locs.";
	}
	else if (commonItems::DoesFolderExist(dirPath / "../localization/english")) // ck3, imp, vic3
	{
		actualPath = dirPath / "../localization/english";
		Log(LogLevel::Info) << "Priming for CK3/Imp/Vic3 Source Locs.";
	}
	else if (commonItems::DoesFolderExist(dirPath / "../../main_menu/localization/english")) // eu5
	{
		actualPath = dirPath / "../../main_menu/localization/english";
		Log(LogLevel::Info) << "Priming for EU5 Source Locs.";
	}
	else if (actualPath.empty())
	{
		Log(LogLevel::Warning) << "No source locs found!";
		return;
	}

	for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
	{
		Log(LogLevel::Debug) << "into file :::::::::: " << (actualPath / fileName).string();
		scrapeFile(actualPath / fileName, LocType::SOURCE);
	}
}

void LocalizationMapper::scrapeTargetDir(const std::filesystem::path& dirPath)
{
	std::filesystem::path actualPath;
	if (commonItems::DoesFolderExist(dirPath / "state_regions")) // vic3 scraping override
	{
		actualPath = dirPath / "state_regions";
		for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
			scrapeFile(actualPath / fileName, LocType::TARGET);
		actualPath.clear();
	}

	if (commonItems::DoesFolderExist(dirPath / "../localisation")) // ck2, eu4, vic2
	{
		actualPath = dirPath / "../localisation";
		Log(LogLevel::Info) << "Priming for Ck2/EU4/Vic2 Target Locs.";
	}
	else if (commonItems::DoesFolderExist(dirPath / "../localization/english")) // ck3, imp, vic3
	{
		actualPath = dirPath / "../localization/english";
		Log(LogLevel::Info) << "Priming for CK3/Imp/Vic3 Target Locs.";
	}
	else if (commonItems::DoesFolderExist(dirPath / "../../main_menu/localization/english")) // eu5
	{
		actualPath = dirPath / "../../main_menu/localization/english";
		Log(LogLevel::Warning) << "No target locs found!";
	}
	else if (actualPath.empty())
	{

		return;
	}

	for (const auto& fileName: commonItems::GetAllFilesInFolderRecursive(actualPath))
		scrapeFile(actualPath / fileName, LocType::TARGET);
}

void LocalizationMapper::scrapeFile(const std::filesystem::path& filePath, LocType locType)
{
	if (filePath.extension() == ".csv")
	{
		const auto locs = CsvScraper(filePath).getLocalizations();
		if (locType == LocType::SOURCE)
			sourceLocalizations.insert(locs.begin(), locs.end());
		else
			targetLocalizations.insert(locs.begin(), locs.end());
	}
	else if (filePath.extension() == ".yml")
	{
		const auto locs = YmlScraper(filePath).getLocalizations();
		if (locType == LocType::SOURCE)
			sourceLocalizations.insert(locs.begin(), locs.end());
		else
			targetLocalizations.insert(locs.begin(), locs.end());
	}
	else if (filePath.extension() == ".txt")
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