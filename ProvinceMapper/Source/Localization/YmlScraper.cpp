#include "YmlScraper.h"
#include "OSCompatibilityLayer.h"
#include <fstream>

YmlScraper::YmlScraper(const std::string& fileName)
{
	std::ifstream fileStream(fileName);
	if (fileStream.is_open())
		scrapeStream(fileStream);
	fileStream.close();
}

void YmlScraper::scrapeStream(std::istream& theStream)
{
	std::string line;
	getline(theStream, line); // This is header line.
	if (line.find("english") == std::string::npos)
		return; // We only want english localizations.

	while (!theStream.eof())
	{
		getline(theStream, line);
		if (line[0] == '#' || line[1] == '#' || line.length() < 4)
			continue;

		const auto sepLoc = line.find(':');
		if (sepLoc == std::string::npos)
			continue;
		const auto key = line.substr(1, sepLoc - 1);
		const auto newLine = line.substr(sepLoc + 1, line.length());
		const auto quoteLoc = newLine.find('\"');
		const auto quote2Loc = newLine.find('\"', quoteLoc + 1);
		if (quoteLoc == std::string::npos || quote2Loc == std::string::npos || quote2Loc - quoteLoc == 0)
			continue;
		auto value = newLine.substr(quoteLoc + 1, quote2Loc - quoteLoc - 1);

		// we're degrading to 1252 because we usually have mix of mapdatanames, and old 1252 locs, all of which are shady.
		value = commonItems::convertUTF8ToWin1252(value);
		localizations[key] = value;
	}
}
