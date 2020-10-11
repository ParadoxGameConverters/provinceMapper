#include "LinkMapper.h"
#include "ParserHelpers.h"
#include <fstream>

#include "Log.h"
#include "../Definitions/Definitions.h"

void LinkMapper::loadMappings(const std::string& fileName, const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerKeys(sourceDefs, targetDefs);
	parseFile(fileName);
	clearRegisteredKeywords();
	if (!versions.empty())
		activeVersion = versions.front();
}

void LinkMapper::registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerRegex(R"(\d+.\d+.\d+)", [this, sourceDefs, targetDefs](const std::string& versionName, std::istream& theStream) {
		const auto version = std::make_shared<LinkMappingVersion>(theStream, versionName, sourceDefs, targetDefs);
		versions.emplace_back(version);
		Log(LogLevel::Info) << "Version " << version->getName() << ", " << version->getLinks()->size() << " links.";
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

void LinkMapper::exportMappings() const
{
	std::ofstream linkFile("test_mappings.txt");
	for (const auto& version: versions)
	{
		linkFile << version->getName() << " = {\n";
		linkFile << *version;
		linkFile << "}\n";		
	}
	linkFile.close();
}
