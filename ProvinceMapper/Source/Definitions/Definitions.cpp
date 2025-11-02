#include "Definitions.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <OSCompatibilityLayer.h>
#include <ParserHelpers.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

namespace
{

std::optional<std::tuple<std::string, unsigned char, unsigned char, unsigned char, std::string>> parseLine(const std::string& line)
{
	try
	{
		auto sepLoc = line.find(';');
		if (sepLoc == std::string::npos)
			return std::nullopt;
		auto sepLocSave = sepLoc;
		auto ID = line.substr(0, sepLoc);
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == std::string::npos)
			return std::nullopt;
		auto r = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == std::string::npos)
			return std::nullopt;
		auto g = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == std::string::npos)
			return std::nullopt;
		auto b = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		std::string mapDataName;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == std::string::npos)
		{
			// There is no closing ;
			// This may be an actual name ot just "x" indicating name isn't defined.
			const auto potentialMapDataName = line.substr(sepLocSave + 1, line.length());
			if (potentialMapDataName != "x" && potentialMapDataName != "X")
				mapDataName = potentialMapDataName;
		}
		else
		{
			// We have a closing ; but name may still be just "x".
			mapDataName = line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1);
			if (mapDataName == "x" || mapDataName == "X")
				mapDataName.clear();
		}
		return std::make_tuple(ID, r, g, b, mapDataName);
	}
	catch (std::exception& e)
	{
		Log(LogLevel::Warning) << "Broken Definition Line: " << line << " - " << e.what();
		return std::nullopt;
	}
}

} // namespace


void Definitions::loadDefinitions(const fs::path& mapDataPath, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	if (!commonItems::DoesFileExist(mapDataPath / "definition.csv"))
		throw std::runtime_error("Definitions file cannot be found!");

	if (commonItems::DoesFileExist(mapDataPath / "area.txt"))
		eu4RegionManager.loadRegions(mapDataPath);

	std::ifstream definitionsFile(mapDataPath / "definition.csv");
	parseStream(definitionsFile, localizationMapper, locType);
	definitionsFile.close();

	tryToLoadProvinceTypes(mapDataPath);
}

void Definitions::parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	std::string line;
	getline(theStream, line); // discard first line.

	// If the first line contains a comment about province 0 being ignored, respect that.
	const bool ignoreProvince0 = line.find("#Province id 0 is ignored") != std::string::npos;

	while (!theStream.eof())
	{
		getline(theStream, line);
		if (!isdigit(line[0]) || line.length() < 4)
			continue;

		try
		{
			const auto& parsedLine = parseLine(line);
			if (parsedLine)
			{
				const auto [ID, r, g, b, mapDataName] = *parsedLine;

				if (ignoreProvince0 && ID == "0")
				{
					continue;
				}

				auto province = std::make_shared<Province>(ID, r, g, b, mapDataName);
				if (locType == LocalizationMapper::LocType::SOURCE)
				{
					// can we get a locname? Probe for PROV first.
					auto locName = localizationMapper.getLocForSourceKey("PROV" + ID);
					if (locName && !locName->empty())
					{
						province->locName = locName;
					}
					else
					{
						// maybe mapdataname as key.
						locName = localizationMapper.getLocForSourceKey(mapDataName);
						if (locName && !locName->empty())
							province->locName = locName;
					}
				}
				else
				{
					// ditto for the other defs.
					auto locName = localizationMapper.getLocForTargetKey("PROV" + ID);
					if (locName && !locName->empty())
					{
						province->locName = locName;
					}
					else
					{
						locName = localizationMapper.getLocForTargetKey(mapDataName);
						if (locName && !locName->empty())
							province->locName = locName;
					}
				}
				if (const auto& regName = eu4RegionManager.getParentAreaName(province->ID); regName)
				{
					const auto& locName = localizationMapper.getLocForSourceKey(*regName);
					if (locName && !locName->empty())
					{
						province->setAreaName(*locName);
					}
					else
						province->setAreaName(*regName);
				}
				if (const auto& regName = eu4RegionManager.getParentRegionName(province->ID); regName)
				{
					const auto& locName = localizationMapper.getLocForSourceKey(*regName);
					if (locName && !locName->empty())
						province->setRegionName(*locName);
					else
						province->setRegionName(*regName);
				}
				if (const auto& regName = eu4RegionManager.getParentSuperRegionName(province->ID); regName)
				{
					const auto& locName = localizationMapper.getLocForSourceKey(*regName);
					if (locName && !locName->empty())
						province->setSuperRegionName(*locName);
					else
						province->setSuperRegionName(*regName);
				}
				provinces.insert(std::pair(province->ID, province));
				chromaCache.insert(std::pair(pixelPack(province->r, province->g, province->b), province));
			}
		}
		catch (std::exception& e)
		{
			throw std::runtime_error("Line: |" + line + "| is unparseable! Breaking. (" + e.what() + ")");
		}
	}
}

void Definitions::registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chromaItr = chromaCache.find(pixelPack(r, g, b));
	if (chromaItr != chromaCache.end())
		chromaItr->second->innerPixels.emplace_back(pixel);
}

void Definitions::registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chromaItr = chromaCache.find(pixelPack(r, g, b));
	if (chromaItr != chromaCache.end())
		chromaItr->second->borderPixels.emplace_back(pixel);
}

std::optional<std::string> Definitions::getNameForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->bespokeName();
	return std::nullopt;
}

std::optional<std::string> Definitions::getMiscForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->miscName();
	return std::nullopt;
}

std::optional<std::string> Definitions::getIDForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second->ID;
	return std::nullopt;
}

std::shared_ptr<Province> Definitions::getProvinceForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		if (chromaCacheItr->second)
			return chromaCacheItr->second;
	return nullptr;
}

std::shared_ptr<Province> Definitions::getProvinceForID(const std::string& ID)
{
	if (const auto& provinceItr = provinces.find(ID); provinceItr != provinces.end())
		if (provinceItr->second)
			return provinceItr->second;
	return nullptr;
}

void Definitions::loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	for (const auto& [id, province]: provinces)
	{
		if (locType == LocalizationMapper::LocType::SOURCE && localizationMapper.getLocForSourceKey(id))
			province->locName = *localizationMapper.getLocForSourceKey(id);
		if (locType == LocalizationMapper::LocType::TARGET && localizationMapper.getLocForTargetKey(id))
			province->locName = *localizationMapper.getLocForTargetKey(id);
	}
}

void Definitions::registerNeighbor(unsigned int provinceChroma, unsigned int neighborChroma)
{
	if (!neighborChromas.contains(provinceChroma))
		neighborChromas.emplace(provinceChroma, std::set<unsigned int>{});
	neighborChromas.at(provinceChroma).emplace(neighborChroma);
}

std::map<unsigned int, std::set<unsigned int>> Definitions::getNeighborChromas() const
{
	return neighborChromas;
}

void Definitions::ditchAdjacencies(const fs::path& fileName)
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

std::string tolower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return str;
}

void Definitions::tryToLoadProvinceTypes(const fs::path& mapDataPath)
{
	const fs::path filePath = mapDataPath / "default.map";
	if (!commonItems::DoesFileExist(filePath))
	{
		return;
	}

	auto parser = commonItems::parser();
	const std::string provinceTypesRegex =
		 "sea_zones|wasteland|impassable_terrain|uninhabitable|river_provinces|lakes|LAKES|impassable_mountains|impassable_seas";
	parser.registerRegex(provinceTypesRegex, [&](const std::string& provinceType, std::istream& stream) {
		std::string lowerCaseProvinceType = tolower(provinceType);

		parser.getNextTokenWithoutMatching(stream); // equals sign

		auto strOfItemStr = commonItems::stringOfItem(stream).getString();
		if (tolower(strOfItemStr) == "list") // format found in Imperator and CK3
		{
			auto provIds = commonItems::getStrings(stream);
			for (auto& id: provIds)
			{
				if (provinces.contains(id) && provinces.at(id))
					provinces[id]->addProvinceType(lowerCaseProvinceType);
			}
		}
		else if (tolower(strOfItemStr) == "range") // format found in Imperator and CK3
		{
			auto provIds = commonItems::getULlongs(stream);
			auto groupSize = provIds.size();
			if (provIds.empty() || groupSize > 2)
			{
				throw new std::runtime_error("A range of provinces should have 1 or 2 elements!");
			}

			auto beginning = provIds[0];
			auto end = provIds[1];
			for (auto id = beginning; id <= end; ++id)
			{
				std::string idStr = std::to_string(id);
				if (provinces.contains(idStr) && provinces.at(idStr))
					provinces[idStr]->addProvinceType(lowerCaseProvinceType);
			}
		}
		else if (strOfItemStr.starts_with("{")) // simple list
		{
			std::stringstream ss;
			ss << strOfItemStr;
			const auto provIds = commonItems::getStrings(ss);

			for (auto& id: provIds)
			{
				provinces[id]->addProvinceType(lowerCaseProvinceType);
			}
		}
		else
		{
			throw std::runtime_error("Unknown province group type: " + strOfItemStr);
		}
	});
	parser.IgnoreAndLogUnregisteredItems();
	parser.parseFile(filePath);
}
