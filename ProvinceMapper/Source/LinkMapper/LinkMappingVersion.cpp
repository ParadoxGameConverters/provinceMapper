#include "LinkMappingVersion.h"
#include "Definitions/Definitions.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"
#include <fstream>

LinkMappingVersion::LinkMappingVersion(std::istream& theStream,
	 std::string theVersionName,
	 std::shared_ptr<Definitions> theSourceDefs,
	 std::shared_ptr<Definitions> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 versionName(std::move(theVersionName)), sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)),
	 targetToken(std::move(theTargetToken)), links(std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>())
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

LinkMappingVersion::LinkMappingVersion(std::string theVersionName,
	 std::shared_ptr<Definitions> theSourceDefs,
	 std::shared_ptr<Definitions> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 versionName(std::move(theVersionName)), sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)),
	 targetToken(std::move(theTargetToken)), links(std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>())
{
}

void LinkMappingVersion::registerKeys()
{
	registerKeyword("link", [this](const std::string& unused, std::istream& theStream) {
		++linkCounter;
		const auto link = std::make_shared<LinkMapping>(theStream, sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
		links->push_back(link);
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

std::optional<int> LinkMappingVersion::toggleProvinceByID(const int provinceID, const bool isSource)
{
	if (activeLink)
	{
		if (isSource)
			activeLink->toggleSource(provinceID);
		else
			activeLink->toggleTarget(provinceID);
		return std::nullopt;
	}
	else
	{
		// Create a new link and activate it.
		const auto link = std::make_shared<LinkMapping>(sourceDefs, targetDefs, sourceToken, targetToken, linkCounter);
		if (isSource)
			link->toggleSource(provinceID);
		else
			link->toggleTarget(provinceID);
		++linkCounter;
		// We're positioning the new link above the last clicked one.
		const auto& positionItr = links->begin() + lastActiveLinkIndex;
		links->insert(positionItr, link);
		activeLink = link;
		return link->getID();
	}
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
