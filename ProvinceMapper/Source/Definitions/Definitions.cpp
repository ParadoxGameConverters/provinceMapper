#include "Definitions.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
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

		ProvinceDefinition definition;
		try
		{
			auto sepLoc = line.find(';');
			if (sepLoc == std::string::npos)
				continue;
			auto sepLocSave = sepLoc;
			definition.provinceID = std::stoi(line.substr(0, sepLoc));
			sepLoc = line.find(';', sepLocSave + 1);
			if (sepLoc == std::string::npos)
				continue;
			definition.r = std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1));
			sepLocSave = sepLoc;
			sepLoc = line.find(';', sepLocSave + 1);
			if (sepLoc == std::string::npos)
				continue;
			definition.g = std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1));
			sepLocSave = sepLoc;
			sepLoc = line.find(';', sepLocSave + 1);
			if (sepLoc == std::string::npos)
				continue;
			definition.b = std::stoi(line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1));
			sepLocSave = sepLoc;
			sepLoc = line.find(';', sepLocSave + 1);
			if (sepLoc == std::string::npos)
				continue;
			definition.mapDataName = line.substr(sepLocSave + 1, sepLoc - sepLocSave - 1);
		}
		catch (std::exception& e)
		{
			throw std::runtime_error("Line: |" + line + "| is unparseable! Breaking. (" + e.what() + ")");
		}
		
		definitions.insert(std::pair(definition.provinceID, definition));
	}
}

std::optional<std::tuple<int, int, int>> Definitions::getColorForProvinceID(int provinceID) const
{
	const auto& definitionItr = definitions.find(provinceID);
	if (definitionItr != definitions.end())
		return std::make_tuple(definitionItr->second.r, definitionItr->second.g, definitionItr->second.b);
	else
		return std::nullopt;
}
