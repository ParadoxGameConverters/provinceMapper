#include "YmlScraper.h"
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
	while (!theStream.eof())
	{
		std::string line;
		getline(theStream, line); // This is header line.
		if (line.find("english") == std::string::npos)
			return; // We only want english localizations.

		if (line[0] == '#' || line[1] == '#' || line.length() < 4)
			continue;

		const auto sepLoc = line.find_first_of(':');
		if (sepLoc == std::string::npos)
			continue;
		const auto key = line.substr(1, sepLoc - 1);
		const auto newLine = line.substr(sepLoc + 1, line.length());
		const auto quoteLoc = newLine.find_first_of('\"');
		const auto quote2Loc = newLine.find_last_of('\"');
		if (quoteLoc == std::string::npos || quote2Loc == std::string::npos || quote2Loc - quoteLoc == 0)
			continue;
		const auto value = newLine.substr(quoteLoc + 1, quote2Loc - quoteLoc - 1);

		localizations[key] = value;
	}
}
