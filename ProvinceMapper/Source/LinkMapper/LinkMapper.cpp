#include "LinkMapper.h"
#include "Definitions/Definitions.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"
#include <fstream>

void LinkMapper::loadMappings(const std::string& linksFileString,
	 const std::shared_ptr<Definitions>& sourceDefs,
	 const std::shared_ptr<Definitions>& targetDefs,
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

void LinkMapper::registerKeys(const std::shared_ptr<Definitions>& sourceDefs,
	 const std::shared_ptr<Definitions>& targetDefs,
	 const std::string& sourceToken,
	 const std::string& targetToken)
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

void LinkMapper::deactivateLink() const
{
	if (activeVersion)
		activeVersion->deactivateLink();
}

void LinkMapper::activateLinkByIndex(const int row) const
{
	if (activeVersion)
		activeVersion->activateLinkByIndex(row);
}

void LinkMapper::activateLinkByID(const int ID) const
{
	if (activeVersion)
		activeVersion->activateLinkByID(ID);
}

std::optional<int> LinkMapper::toggleProvinceByID(const int provinceID, const bool isSource) const
{
	if (activeVersion)
		return activeVersion->toggleProvinceByID(provinceID, isSource);
	else
		return std::nullopt;
}
