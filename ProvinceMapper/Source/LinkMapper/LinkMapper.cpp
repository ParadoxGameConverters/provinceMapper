#include "LinkMapper.h"
#include "ParserHelpers.h"
#include <fstream>

#include "Definitions/Definitions.h"
#include "Log.h"

void LinkMapper::loadMappings(const std::string& linksFileString,
	 const Definitions& sourceDefs,
	 const Definitions& targetDefs,
	 const std::string& sourceToken,
	 const std::string& targetToken)
{
	registerKeys(sourceDefs, targetDefs, sourceToken, targetToken);
	std::stringstream linksStream(linksFileString);
	parseStream(linksStream);
	clearRegisteredKeywords();
	if (!versions.empty())
		activeVersion = versions.front();
}

void LinkMapper::registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs, const std::string& sourceToken, const std::string& targetToken)
{
	registerRegex(R"(\d+.\d+.\d+)", [this, sourceDefs, targetDefs, sourceToken, targetToken](const std::string& versionName, std::istream& theStream) {
		const auto version = std::make_shared<LinkMappingVersion>(theStream, versionName, sourceDefs, targetDefs, sourceToken, targetToken);
		versions.emplace_back(version);
		Log(LogLevel::Info) << "Version " << version->getName() << ", " << version->getLinks()->size() << " links.";
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

void LinkMapper::exportMappings(const std::string& linksFile) const
{
	std::ofstream linkFile(linksFile);
	for (const auto& version: versions)
	{
		linkFile << *version;
	}
	linkFile.close();
}
