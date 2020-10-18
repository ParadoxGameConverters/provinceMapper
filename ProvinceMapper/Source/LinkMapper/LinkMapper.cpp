#include "LinkMapper.h"
#include "Definitions/Definitions.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"
#include <fstream>

void LinkMapper::loadMappings(const std::string& linksFileString,
	 std::shared_ptr<Definitions> theSourceDefs,
	 std::shared_ptr<Definitions> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken)	 
{
	sourceDefs = std::move(theSourceDefs);
	targetDefs = std::move(theTargetDefs);
	sourceToken = std::move(theSourceToken);
	targetToken = std::move(theTargetToken);
	
	registerKeys();
	std::stringstream linksStream(linksFileString);
	parseStream(linksStream);
	clearRegisteredKeywords();
	if (versions.empty())
	{
		auto newVersion = std::make_shared<LinkMappingVersion>("0.0.0", sourceDefs, targetDefs, sourceToken, targetToken);
		versions.emplace_back(newVersion);
		Log(LogLevel::Info) << "Generated version " << newVersion->getName() << ", " << newVersion->getLinks()->size() << " links.";
	}
	activeVersion = versions.front();
}

void LinkMapper::registerKeys()
{
	registerRegex(R"(\d+.\d+.\d+)", [this](const std::string& versionName, std::istream& theStream) {
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

std::optional<int> LinkMapper::addCommentByIndex(const std::string& comment, const int index) const
{
	if (activeVersion)
		return activeVersion->addCommentByIndex(comment, index);
	else
		return std::nullopt;
}

void LinkMapper::deleteActiveLink() const
{
	if (activeVersion)
		activeVersion->deleteActiveLink();
}

std::optional<int> LinkMapper::addRawLink() const
{
	if (activeVersion)
		return activeVersion->addRawLink();
	else
		return std::nullopt;
}

std::optional<int> LinkMapper::addRawComment() const
{
	if (activeVersion)
		return activeVersion->addRawComment();
	else
		return std::nullopt;
}
