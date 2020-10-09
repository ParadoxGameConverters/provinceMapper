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
	registerKeys();
	parseFile("test_mappings.txt");
	clearRegisteredKeywords();
	if (!versions.empty())
		activeVersion = versions.rbegin()->second;
}

void LinkMapper::registerKeys()
{
	registerRegex(R"(\d+.\d+.\d+)", [this](const std::string& versionName, std::istream& theStream) {
		const auto version = std::make_shared<LinkMappingVersion>(theStream);
		versions.insert(std::pair(versionName, version));
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

void LinkMapper::exportMappings() const
{
	std::ofstream linkFile("test_mappings.txt");
	for (const auto& [versionName, version]: versions)
	{
		linkFile << versionName << " = {\n";
		linkFile << *version;
		linkFile << "}\n";		
	}
	linkFile.close();
}
