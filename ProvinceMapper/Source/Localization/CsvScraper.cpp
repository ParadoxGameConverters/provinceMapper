#include "CsvScraper.h"
#include <fstream>

CsvScraper::CsvScraper(const std::string& fileName)
{
	std::ifstream theFile(fileName);
	if (theFile.is_open())
		scrapeStream(theFile);
	theFile.close();
}

void CsvScraper::scrapeStream(std::istream& theStream)
{
	while (!theStream.eof())
	{
		std::string line;
		getline(theStream, line);

		if (line[0] == '#' || line.length() < 4)
			continue;

		auto sepLoc = line.find_first_of(';');
		auto key = line.substr(0, sepLoc);
		auto newLine = line.substr(sepLoc + 1, line.length());
		sepLoc = newLine.find_first_of(';');
		const auto english = newLine.substr(0, sepLoc);

		localizations[key] = english;
	}
}
