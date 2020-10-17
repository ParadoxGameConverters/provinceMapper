#include "Definitions.h"
#include "OSCompatibilityLayer.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <fstream>

void Definitions::loadDefinitions(std::istream& theStream)
{
	parseStream(theStream);
}

void Definitions::loadDefinitions(const std::string& fileName)
{
	if (!commonItems::DoesFileExist(fileName))
		throw std::runtime_error("Definitions file cannot be found!");

	std::ifstream definitionsFile(fileName);
	parseStream(definitionsFile);
	definitionsFile.close();
}

void Definitions::parseStream(std::istream& theStream)
{
	std::string line;
	getline(theStream, line); // discard first line.

	while (!theStream.eof())
	{
		getline(theStream, line);
		if (line[0] == '#' || line[1] == '#' || line.length() < 4)
			continue;

		try
		{
			const auto& parsedLine = parseLine(line);
			if (parsedLine)
			{
				const auto [ID, r, g, b, mapDataName] = *parsedLine;
				auto province = std::make_shared<Province>(ID, r, g, b, mapDataName);
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

std::optional<std::tuple<int, unsigned char, unsigned char, unsigned char, std::string>> Definitions::parseLine(const std::string& line)
{
	auto sepLoc = line.find(';');
	if (sepLoc == std::string::npos)
		return std::nullopt;
	auto sepLocSave = sepLoc;
	auto ID = std::stoi(line.substr(0, sepLoc));
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
		if (chromaCacheItr->second->locName.empty())
			return chromaCacheItr->second->mapDataName;
		else
			return chromaCacheItr->second->locName;
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

std::shared_ptr<Province> Definitions::getProvinceForID(const int ID)
{
	if (const auto& provinceItr = provinces.find(ID); provinceItr != provinces.end())
		return provinceItr->second;
	else
		return nullptr;
}
