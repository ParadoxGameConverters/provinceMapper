#include "LinkMappingVersion.h"
#include "ParserHelpers.h"

LinkMappingVersion::LinkMappingVersion(std::istream& theStream)
{
	links = std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>();
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void LinkMappingVersion::registerKeys()
{
	registerKeyword("link", [this](const std::string& unused, std::istream& theStream) {
		const auto link = std::make_shared<LinkMapping>(theStream);
		links->push_back(link);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion)
{
	for (const auto& link: *linkMappingVersion.links)
		output << *link;
	return output;
}
