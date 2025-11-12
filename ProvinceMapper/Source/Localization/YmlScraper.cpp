#include "YmlScraper.h"
#include <OSCompatibilityLayer.h>
#include <fstream>

YmlScraper::YmlScraper(const std::filesystem::path& fileName)
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
		if (line.length() < 4 || line[0] == '#' || line[1] == '#')
			continue;

		const auto sepLoc = line.find(':');
		if (sepLoc == std::string::npos)
			continue;

		// first character (whitespace) may or may not be present due to bugs.
		const std::string delims(" \t");
		auto keyStart = line.find_first_not_of(delims); // We're ok not testing as we know there's a : somewhere in there.
		const std::string key = line.substr(keyStart, sepLoc - keyStart);

		const auto newLine = line.substr(sepLoc + 1, line.length());
		const auto quoteLoc = newLine.find('\"');
		const auto quote2Loc = newLine.find('\"', quoteLoc + 1);
		if (quoteLoc == std::string::npos || quote2Loc == std::string::npos || quote2Loc - quoteLoc == 0)
			continue;
		auto value = newLine.substr(quoteLoc + 1, quote2Loc - quoteLoc - 1);

		// we're degrading to 1252 because we usually have mix of mapdatanames, and old 1252 locs, all of which are shady.
		// There is no downside degrading pure UTF8 sources like EU5, more ascii-like the better.
		value = commonItems::convertUTF8ToASCII(value);

		localizations[key] = value;
	}
}
