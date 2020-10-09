#include "LinkMapper.h"
#include "ParserHelpers.h"
#include <fstream>

LinkMapper::LinkMapper(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

LinkMapper::LinkMapper()
{
	links = std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>();
	registerKeys();
	parseFile("test_mappings.txt");
	clearRegisteredKeywords();
}

void LinkMapper::registerKeys()
{
	registerKeyword("link", [this](const std::string& unused, std::istream& theStream) {
		const auto link = std::make_shared<LinkMapping>(theStream);
		links->push_back(link);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

void LinkMapper::exportLinks() const
{
	std::ofstream linkFile("test_mappings.txt");
	for (const auto& link: *links)
		linkFile << *link;
	linkFile.close();
}
