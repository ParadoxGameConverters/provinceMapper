#include "LinkMappingVersion.h"
#include "CommonRegexes.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"
#include <fstream>
#include <set>

LinkMappingVersion::LinkMappingVersion(std::istream& theStream,
	 std::string theVersionName,
	 std::shared_ptr<DefinitionsInterface> theSourceDefs,
	 std::shared_ptr<DefinitionsInterface> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 versionName(std::move(theVersionName)), sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)),
	 targetToken(std::move(theTargetToken)), 
	 triangulationPairs(std::make_shared<std::vector<std::shared_ptr<TriangulationPointPair>>>()),
	 links(std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>()),
	 unmappedSources(std::make_shared<std::vector<std::shared_ptr<Province>>>()), unmappedTargets(std::make_shared<std::vector<std::shared_ptr<Province>>>())
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
	generateUnmapped();
}

LinkMappingVersion::LinkMappingVersion(std::string theVersionName,
	 std::shared_ptr<DefinitionsInterface> theSourceDefs,
	 std::shared_ptr<DefinitionsInterface> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 versionName(std::move(theVersionName)), sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)),
	 targetToken(std::move(theTargetToken)),
	 triangulationPairs(std::make_shared<std::vector<std::shared_ptr<TriangulationPointPair>>>()),
	 links(std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>()),
	 unmappedSources(std::make_shared<std::vector<std::shared_ptr<Province>>>()), unmappedTargets(std::make_shared<std::vector<std::shared_ptr<Province>>>())
{
	generateUnmapped();
}

void LinkMappingVersion::registerKeys()
{	
	registerKeyword("triangulation_pair", [this](std::istream& theStream) {
		++triangulationPairCounter;
		const auto pair = std::make_shared<TriangulationPointPair>(theStream, linkCounter);
		triangulationPairs->push_back(pair);
	});
	registerKeyword("link", [this](std::istream& theStream) {
		++linkCounter;
		const auto link = std::make_shared<LinkMapping>(theStream, sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
		links->push_back(link);
		for (const auto& source: link->getSources())
		{
			if (seenSources.contains(source->ID))
				Log(LogLevel::Error) << "Source province " << source->ID << " is double-mapped!";
			else
				seenSources.emplace(source->ID);
		}
		for (const auto& target: link->getTargets())
		{
			if (seenTargets.contains(target->ID))
				Log(LogLevel::Error) << "Target province " << target->ID << " is double-mapped!";
			else
				seenTargets.emplace(target->ID);
		}
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion)
{
	output << linkMappingVersion.versionName << " = {\n";
	for (const auto& link: *linkMappingVersion.links)
		output << *link;
	output << "}\n";
	return output;
}

void LinkMappingVersion::deactivateLink()
{
	if (activeLink)
	{
		if (activeLink->getSources().empty() && activeLink->getTargets().empty() && !activeLink->getComment())
			deleteActiveLink();
	}
	activeLink.reset();
}

void LinkMappingVersion::activateLinkByIndex(const int row)
{
	if (row < static_cast<int>(links->size()))
	{
		activeLink = links->at(row);
		lastActiveLinkIndex = row;
	}
}

void LinkMappingVersion::activateLinkByID(const int theID)
{
	auto counter = 0;
	for (const auto& link: *links)
	{
		if (link->getID() == theID)
		{
			activeLink = link;
			lastActiveLinkIndex = counter;
			break;
		}
		++counter;
	}
}

std::optional<int> LinkMappingVersion::toggleProvinceByID(const std::string& provinceID, const bool isSource)
{
	if (activeLink)
	{
		if (isSource)
		{
			const auto result = activeLink->toggleSource(provinceID);
			if (result == Mapping::MAPPED)
				removeUnmappedSourceByID(provinceID);
			else if (result == Mapping::UNMAPPED)
				addUnmappedSourceByID(provinceID);
		}
		else
		{
			const auto result = activeLink->toggleTarget(provinceID);
			if (result == Mapping::MAPPED)
				removeUnmappedTargetByID(provinceID);
			else if (result == Mapping::UNMAPPED)
				addUnmappedTargetByID(provinceID);
		}
		return std::nullopt;
	}
	else
	{
		// Create a new link and activate it.
		const auto link = std::make_shared<LinkMapping>(sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
		if (isSource)
		{
			const auto result = link->toggleSource(provinceID);
			if (result == Mapping::MAPPED)
				removeUnmappedSourceByID(provinceID);
		}
		else
		{
			const auto result = link->toggleTarget(provinceID);
			if (result == Mapping::MAPPED)
				removeUnmappedTargetByID(provinceID);
		}
		++linkCounter;
		// We're positioning the new link above the last clicked one.
		const auto& positionItr = links->begin() + lastActiveLinkIndex;
		links->insert(positionItr, link);
		activeLink = link;
		return link->getID();
	}
}

void LinkMappingVersion::removeUnmappedSourceByID(const std::string& provinceID) const
{
	auto counter = 0;
	for (const auto& province: *unmappedSources)
	{
		if (province->ID == provinceID)
		{
			(*unmappedSources).erase((*unmappedSources).begin() + counter);
			break;
		}
		++counter;
	}
}

void LinkMappingVersion::removeUnmappedTargetByID(const std::string& provinceID) const
{
	auto counter = 0;
	for (const auto& province: *unmappedTargets)
	{
		if (province->ID == provinceID)
		{
			(*unmappedTargets).erase((*unmappedTargets).begin() + counter);
			break;
		}
		++counter;
	}
}

void LinkMappingVersion::addUnmappedSourceByID(const std::string& provinceID) const
{
	const auto& province = sourceDefs->getProvinceForID(provinceID);
	unmappedSources->emplace_back(province);
}

void LinkMappingVersion::addUnmappedTargetByID(const std::string& provinceID) const
{
	const auto& province = targetDefs->getProvinceForID(provinceID);
	unmappedTargets->emplace_back(province);
}

int LinkMappingVersion::addCommentByIndex(const std::string& comment, const int index)
{
	// Create a new link with the comment.
	const auto link = std::make_shared<LinkMapping>(sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
	link->setComment(comment);
	++linkCounter;
	const auto& positionItr = links->begin() + index;
	links->insert(positionItr, link);
	activeLink = link;
	lastActiveLinkIndex = index;
	return link->getID();
}

void LinkMappingVersion::deleteActiveLink()
{
	if (activeLink)
	{
		auto counter = 0;
		for (const auto& link: *links)
		{
			if (*link == *activeLink)
			{
				// Move all mapped provinces to unmapped.
				for (const auto& province: link->getSources())
					unmappedSources->emplace_back(province);
				for (const auto& province: link->getTargets())
					unmappedTargets->emplace_back(province);
				// we're deleting it.
				links->erase((*links).begin() + counter);
				break;
			}
			++counter;
		}
		activeLink.reset();
	}
}

int LinkMappingVersion::addRawLink()
{
	// Create a new link
	const auto link = std::make_shared<LinkMapping>(sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
	++linkCounter;
	const auto& positionItr = links->begin() + lastActiveLinkIndex;
	links->insert(positionItr, link);
	activeLink = link;
	return link->getID();
}

int LinkMappingVersion::addRawComment()
{
	// Create a new link with the comment.
	const auto link = std::make_shared<LinkMapping>(sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
	link->setComment("");
	++linkCounter;
	const auto& positionItr = links->begin() + lastActiveLinkIndex;
	links->insert(positionItr, link);
	activeLink = link;
	return link->getID();
}

int LinkMappingVersion::addRawTriangulationPair()
{
	// Create a new pair.
	const auto pair = std::make_shared<TriangulationPointPair>(triangulationPairCounter);
	++triangulationPairCounter;
	const auto& positionItr = triangulationPairs->begin() + lastActiveTriangulationPairIndex;
	triangulationPairs->insert(positionItr, pair);
	activeTriangulationPair = pair;
	return pair->getID();
}

void LinkMappingVersion::moveActiveLinkUp() const
{
	if (activeLink)
	{
		size_t counter = 0;
		for (const auto& link: *links)
		{
			if (*link == *activeLink && counter > 0)
			{
				std::swap((*links)[counter], (*links)[counter - 1]);
				break;
			}
			++counter;
		}
	}
}

void LinkMappingVersion::moveActiveLinkDown() const
{
	if (activeLink)
	{
		size_t counter = 0;
		for (const auto& link: *links)
		{
			if (*link == *activeLink && counter < links->size() - 1)
			{
				std::swap((*links)[counter], (*links)[counter + 1]);
				break;
			}
			++counter;
		}
	}
}

bool LinkMappingVersion::operator==(const LinkMappingVersion& rhs) const
{
	return ID == rhs.ID;
}

void LinkMappingVersion::generateUnmapped() const
{
	std::set<std::string> mappedSources;
	std::set<std::string> mappedTargets;
	for (const auto& link: *links)
	{
		if (link->getComment())
			continue;
		for (const auto& source: link->getSources())
			mappedSources.insert(source->ID);
		for (const auto& target: link->getTargets())
			mappedTargets.insert(target->ID);
	}
	for (const auto& [id, province]: sourceDefs->getProvinces())
	{
		// normal provinces have a mapdata name, at least. Plenty of unnamed reserve provinces we don't need.
		if (province->mapDataName.empty())
			continue;
		if (!mappedSources.contains(id))
			unmappedSources->emplace_back(province);
	}
	for (const auto& [id, province]: targetDefs->getProvinces())
	{
		if (province->mapDataName.empty())
			continue;
		if (!mappedTargets.contains(id))
			unmappedTargets->emplace_back(province);
	}
	Log(LogLevel::Info) << "Version " << versionName << " has " << unmappedSources->size() << " unmapped source, " << unmappedTargets->size()
							  << " unmapped target provinces.";
}

Mapping LinkMappingVersion::isProvinceMapped(const std::string& provinceID, bool isSource) const
{
	if (isSource)
	{
		for (const auto& province: *unmappedSources)
			if (province->ID == provinceID)
				return Mapping::UNMAPPED;
		for (const auto& link: *links)
			for (const auto& province: link->getSources())
				if (province->ID == provinceID)
					return Mapping::MAPPED;
	}
	else
	{
		for (const auto& province: *unmappedTargets)
			if (province->ID == provinceID)
				return Mapping::UNMAPPED;
		for (const auto& link: *links)
			for (const auto& province: link->getTargets())
				if (province->ID == provinceID)
					return Mapping::MAPPED;
	}
	return Mapping::FAIL;
}
