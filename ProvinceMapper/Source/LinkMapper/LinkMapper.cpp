#include "LinkMapper.h"
#include "CommonRegexes.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"
#include <fstream>

void LinkMapper::loadMappings(const std::string& linksFileString,
	 std::shared_ptr<DefinitionsInterface> theSourceDefs,
	 std::shared_ptr<DefinitionsInterface> theTargetDefs,
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
		auto newVersion = std::make_shared<LinkMappingVersion>("0.0.0", sourceDefs, targetDefs, sourceToken, targetToken, versionCounter);
		++versionCounter;
		versions.emplace_back(newVersion);
		Log(LogLevel::Info) << "Generated version " << newVersion->getName() << ", " << newVersion->getLinks()->size() << " links.";
	}
	activeVersion = versions.front();
}

void LinkMapper::registerKeys()
{
	registerRegex(commonItems::stringRegex, [this](const std::string& versionName, std::istream& theStream) {
		const auto version = std::make_shared<LinkMappingVersion>(theStream, versionName, sourceDefs, targetDefs, sourceToken, targetToken, versionCounter);
		++versionCounter;
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

void LinkMapper::deactivateTriangulationPair() const
{
	if (activeVersion)
		activeVersion->deactivateTriangulationPair();
}

void LinkMapper::activateLinkByIndex(const int row) const
{
	if (activeVersion)
		activeVersion->activateLinkByIndex(row);
}

void LinkMapper::activateTriangulationPairByIndex(const int row) const
{
	if (activeVersion)
	{
		activeVersion->activateTriangulationPairByIndex(row);
	}
}

void LinkMapper::activateLinkByID(const int ID) const
{
	if (activeVersion)
		activeVersion->activateLinkByID(ID);
}

std::optional<int> LinkMapper::toggleProvinceByID(const std::string& provinceID, const bool isSource) const
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

void LinkMapper::deleteActiveTriangulationPair() const
{
	if (activeVersion)
		activeVersion->deleteActiveTriangulationPair();
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

std::optional<int> LinkMapper::addRawTriangulationPair() const
{
	if (activeVersion)
		return activeVersion->addRawTriangulationPair();
	else
		return std::nullopt;
}

const std::shared_ptr<LinkMappingVersion>& LinkMapper::addVersion()
{
	std::string name = "0.0.0";
	// Naming matters. Do we have a version named "0.0.0"?
	for (const auto& version: versions)
		if (version->getName() == "0.0.0")
		{
			name += "-(RENAME ME " + std::to_string(versionCounter) + ")";
			break;
		}
	const auto newVersion = std::make_shared<LinkMappingVersion>(name, sourceDefs, targetDefs, sourceToken, targetToken, versionCounter);
	++versionCounter;
	versions.insert(versions.begin(), newVersion); // Yes, Front!
	activeVersion = newVersion;
	return activeVersion;
}

const std::shared_ptr<LinkMappingVersion>& LinkMapper::copyVersion()
{
	if (activeVersion)
	{
		auto name = activeVersion->getName();
		name += "-(RENAME ME " + std::to_string(versionCounter) + ")";
		const auto newVersion = std::make_shared<LinkMappingVersion>(name, sourceDefs, targetDefs, sourceToken, targetToken, versionCounter);
		++versionCounter;
		newVersion->copyLinks(activeVersion->getLinks());
		versions.insert(versions.begin(), newVersion);
		activeVersion = newVersion;
	}
	else
	{
		return addVersion();
	}
	return activeVersion;
}

const std::shared_ptr<LinkMappingVersion>& LinkMapper::deleteVersion()
{
	if (activeVersion)
	{
		auto counter = 0;
		for (const auto& version: versions)
		{
			if (version->getID() == activeVersion->getID())
			{
				versions.erase(versions.begin() + counter);
				break;
			}
			++counter;
		}
	}
	if (versions.empty())
	{
		return addVersion();
	}
	else
	{
		activeVersion = versions.front();
		return activeVersion;
	}
}

void LinkMapper::updateActiveVersionName(const std::string& theName) const
{
	if (activeVersion)
	{
		activeVersion->setName(theName);
	}
}

const std::shared_ptr<LinkMappingVersion>& LinkMapper::activateVersionByIndex(const int index)
{
	activeVersion = versions[index];
	return activeVersion;
}

void LinkMapper::moveActiveLinkUp() const
{
	if (activeVersion)
		activeVersion->moveActiveLinkUp();
}

void LinkMapper::moveActiveLinkDown() const
{
	if (activeVersion)
		activeVersion->moveActiveLinkDown();
}

void LinkMapper::moveActiveVersionLeft()
{
	if (activeVersion)
	{
		size_t counter = 0;
		for (const auto& version: versions)
		{
			if (*version == *activeVersion && counter > 0)
			{
				std::swap(versions[counter], versions[counter - 1]);
				break;
			}
			++counter;
		}
	}
}

void LinkMapper::moveActiveVersionRight()
{
	if (activeVersion)
	{
		size_t counter = 0;
		for (const auto& version: versions)
		{
			if (*version == *activeVersion && counter < versions.size() - 1)
			{
				std::swap(versions[counter], versions[counter + 1]);
				break;
			}
			++counter;
		}
	}
}

void LinkMapper::autogenerateMappings()
{
	if (activeVersion){
		activeVersion->autogenerateMappings();
	}
}

Mapping LinkMapper::isProvinceMapped(const std::string& provinceID, bool isSource) const
{
	if (activeVersion)
		return activeVersion->isProvinceMapped(provinceID, isSource);
	else
		return Mapping::FAIL;
}
