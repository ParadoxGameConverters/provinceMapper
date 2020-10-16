#include "LinkMappingVersion.h"
#include "Definitions/Definitions.h"
#include "ParserHelpers.h"
#include <fstream>

LinkMappingVersion::LinkMappingVersion(std::istream& theStream,
	 std::string theVersionName,
	 const Definitions& sourceDefs,
	 const Definitions& targetDefs,
	 const std::string& sourceToken,
	 const std::string& targetToken):
	 versionName(std::move(theVersionName)),
	 links(std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>())
{
	registerKeys(sourceDefs, targetDefs, sourceToken, targetToken);
	parseStream(theStream);
	clearRegisteredKeywords();
}

void LinkMappingVersion::registerKeys(const Definitions& sourceDefs,
	 const Definitions& targetDefs,
	 const std::string& sourceToken,
	 const std::string& targetToken)
{
	registerKeyword("link", [this, sourceDefs, targetDefs, sourceToken, targetToken](const std::string& unused, std::istream& theStream) {
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
	activeLink.reset();
}

void LinkMappingVersion::activateLinkByIndex(const int row)
{
	if (row < static_cast<int>(links->size()))
		activeLink = links->at(row);
}

void LinkMappingVersion::activateLinkByID(const int ID)
{
	for (const auto& link: *links)
		if (link->getID() == ID)
		{
			activeLink = link;
			break;
		}
}
