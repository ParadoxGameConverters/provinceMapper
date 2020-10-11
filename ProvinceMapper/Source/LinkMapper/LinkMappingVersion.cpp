#include "LinkMappingVersion.h"
#include "ParserHelpers.h"
#include "../Definitions/Definitions.h"

LinkMappingVersion::LinkMappingVersion(std::istream& theStream, const std::string& theVersionName, const Definitions& sourceDefs, const Definitions& targetDefs)
{
	versionName = theVersionName;
	links = std::make_shared<std::vector<std::shared_ptr<LinkMapping>>>();
	registerKeys(sourceDefs, targetDefs);
	parseStream(theStream);
	clearRegisteredKeywords();
}

void LinkMappingVersion::registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerKeyword("link", [this, sourceDefs, targetDefs](const std::string& unused, std::istream& theStream) {
		const auto link = std::make_shared<LinkMapping>(theStream, sourceDefs, targetDefs);
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
