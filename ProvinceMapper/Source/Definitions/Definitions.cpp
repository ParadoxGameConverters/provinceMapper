#include "Definitions.h"
#include "OSCompatibilityLayer.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <fstream>
#include <filesystem>
#include <ParserHelpers.h>
namespace fs = std::filesystem;

using std::string;
using std::optional;
using std::nullopt;
using std::shared_ptr;
using std::tuple;
using std::map;
using std::set;
using std::ofstream;
using std::exception;
using std::runtime_error;

namespace
{

optional<tuple<string, unsigned char, unsigned char, unsigned char, string>> parseLine(const string& line)
{
	try
	{
		auto sepLoc = line.find(';');
		if (sepLoc == string::npos)
			return nullopt;
		auto sepLocSave = sepLoc;
		auto ID = line.substr(0, sepLoc);
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == string::npos)
			return nullopt;
		auto r = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == string::npos)
			return nullopt;
		auto g = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == string::npos)
			return nullopt;
		auto b = static_cast<unsigned char>(std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1)));
		sepLocSave = sepLoc;
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == string::npos)
			return nullopt;
		auto mapDataName = line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1);
		return std::make_tuple(ID, r, g, b, mapDataName);
	}
	catch (exception& e)
	{
		Log(LogLevel::Warning) << "Broken Definition Line: " << line << " - " << e.what();
		return nullopt;
	}
}

} // namespace


void Definitions::loadDefinitions(const string& mapDataPath, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	if (!commonItems::DoesFileExist(mapDataPath + "/definition.csv"))
		throw runtime_error("Definitions file cannot be found!");

	if (commonItems::DoesFileExist(mapDataPath + "/area.txt"))
		eu4RegionManager.loadRegions(mapDataPath);

	std::ifstream definitionsFile(mapDataPath + "/definition.csv");
	parseStream(definitionsFile, localizationMapper, locType);
	definitionsFile.close();

	tryToLoadProvinceTypes(mapDataPath);
}

void Definitions::parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	string line;
	getline(theStream, line); // discard first line.

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
						province->areaName = *locName;
					}
					else
						province->areaName = *regName;
				}
				if (const auto& regName = eu4RegionManager.getParentRegionName(province->ID); regName)
				{
					const auto& locName = localizationMapper.getLocForSourceKey(*regName);
					if (locName && !locName->empty())
						province->regionName = *locName;
					else
						province->regionName = *regName;
				}
				if (const auto& regName = eu4RegionManager.getParentSuperRegionName(province->ID); regName)
				{
					const auto& locName = localizationMapper.getLocForSourceKey(*regName);
					if (locName && !locName->empty())
						province->superRegionName = *locName;
					else
						province->superRegionName = *regName;
				}
				provinces.insert(std::pair(province->ID, province));
				chromaCache.insert(std::pair(pixelPack(province->r, province->g, province->b), province));
			}
		}
		catch (exception& e)
		{
			throw runtime_error("Line: |" + line + "| is unparseable! Breaking. (" + e.what() + ")");
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

optional<string> Definitions::getNameForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second->bespokeName();
	else
		return nullopt;
}

optional<string> Definitions::getMiscForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second->miscName();
	else
		return nullopt;
}

optional<string> Definitions::getIDForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second->ID;
	else
		return nullopt;
}

shared_ptr<Province> Definitions::getProvinceForChroma(const unsigned int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second;
	else
		return nullptr;
}

shared_ptr<Province> Definitions::getProvinceForID(const string& ID)
{
	if (const auto& provinceItr = provinces.find(ID); provinceItr != provinces.end())
		return provinceItr->second;
	else
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
		neighborChromas.emplace(provinceChroma, set<unsigned int>{});
	neighborChromas.at(provinceChroma).emplace(neighborChroma);
}

map<unsigned int, set<unsigned int>> Definitions::getNeighborChromas() const
{
	return neighborChromas;
}

void Definitions::ditchAdjacencies(const string& fileName)
{
	map<string, set<string>> adjacencies;
	for (const auto& [sourceChroma, targetChromas]: neighborChromas)
	{
		if (const auto& sourceProvince = getIDForChroma(sourceChroma); sourceChroma)
		{
			adjacencies.emplace(*sourceProvince, set<string>{});
			for (const auto& targetChroma: targetChromas)
			{
				if (const auto& targetProvince = getIDForChroma(targetChroma); targetProvince)
					adjacencies.at(*sourceProvince).emplace(*targetProvince);
			}
		}
	}
	ofstream adjacenciesFile(fileName);
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

void Definitions::tryToLoadProvinceTypes(const string& mapDataPath)
{
	fs::path filePath = fs::path(mapDataPath) / fs::path("default.map");
	auto filePathStr = filePath.string();
	if (!commonItems::DoesFileExist(filePathStr))
	{
		return;
	}

	auto parser = commonItems::parser();
	const string provinceTypesRegex = "sea_zones|wasteland|impassable_terrain|uninhabitable|river_provinces|lakes|LAKES|impassable_mountains|impassable_seas";
	parser.registerRegex(provinceTypesRegex, [&](const std::string& provinceType, std::istream& stream) {
		parser.getNextTokenWithoutMatching(stream); // equals sign
		auto typeOfGroup = parser.getNextTokenWithoutMatching(stream).value();
		auto provIds = commonItems::getULlongs(stream);

		if (typeOfGroup == "RANGE")
		{	
			auto groupSize = provIds.size();
			if (provIds.size() < 1 || groupSize > 2)
			{
				throw new runtime_error("A range of provinces should have 1 or 2 elements!");
			}

			auto beginning = provIds[0];
			auto end = provIds[1];
			for (auto id = beginning; id <= end; ++id)
			{
				string idStr = std::to_string(id);
				provinces[idStr]->provinceType = provinceType;
			}
		}
		else if (typeOfGroup == "LIST")
		{
			for (auto id : provIds)
			{
				string idStr = std::to_string(id);
				provinces[idStr]->provinceType = provinceType;
			}
		}
		else
		{
			throw runtime_error("Unknown province group type: " + typeOfGroup);
		}
	});
	parser.IgnoreAndLogUnregisteredItems();
	parser.parseFile(filePathStr);
}
