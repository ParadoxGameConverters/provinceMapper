#include "LinkMapper.h"
#include "ParserHelpers.h"
#include <fstream>

void LinkMapper::loadMappings(const std::string& fileName, const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerKeys(sourceDefs, targetDefs);
	parseFile(fileName);
	clearRegisteredKeywords();
	if (!versions.empty())
		activeVersion = versions.rbegin()->second;
}

void LinkMapper::registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerRegex(R"(\d+.\d+.\d+)", [this, sourceDefs, targetDefs](const std::string& versionName, std::istream& theStream) {
		const auto version = std::make_shared<LinkMappingVersion>(theStream, sourceDefs, targetDefs);
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
