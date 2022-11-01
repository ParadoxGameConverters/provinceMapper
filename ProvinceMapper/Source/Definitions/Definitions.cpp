#include "Definitions.h"
#include "OSCompatibilityLayer.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <fstream>



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
		sepLoc = line.find(';', sepLocSave + 1);
		if (sepLoc == std::string::npos)
			return std::nullopt;
		auto mapDataName = line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1);
		return std::make_tuple(ID, r, g, b, mapDataName);
	}
	catch (std::exception& e)
	{
		Log(LogLevel::Warning) << "Broken Definition Line: " << line << " - " << e.what();
		return std::nullopt;
	}
}

} // namespace


void Definitions::loadDefinitions(const std::string& fileName, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	if (!commonItems::DoesFileExist(fileName))
		throw std::runtime_error("Definitions file cannot be found!");

	std::ifstream definitionsFile(fileName);
	parseStream(definitionsFile, localizationMapper, locType);
	definitionsFile.close();
}

void Definitions::parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	std::string line;
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

std::optional<std::string> Definitions::getNameForChroma(const int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second->bespokeName();
	else
		return std::nullopt;
}

std::optional<std::string> Definitions::getIDForChroma(const int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second->ID;
	else
		return std::nullopt;
}

std::shared_ptr<Province> Definitions::getProvinceForChroma(const int chroma)
{
	if (const auto& chromaCacheItr = chromaCache.find(chroma); chromaCacheItr != chromaCache.end())
		return chromaCacheItr->second;
	else
		return nullptr;
}

std::shared_ptr<Province> Definitions::getProvinceForID(const std::string& ID)
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
