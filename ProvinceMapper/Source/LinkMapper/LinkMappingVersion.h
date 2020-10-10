#ifndef LINK_MAPPING_VERSION_H
#define LINK_MAPPING_VERSION_H

#include "LinkMapping.h"
#include "Parser.h"

class Definitions;
class LinkMappingVersion: commonItems::parser
{
  public:
	LinkMappingVersion(std::istream& theStream, const Definitions& sourceDefs, const Definitions& targetDefs);

	[[nodiscard]] const auto& getLinks() const { return links; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

  private:
	void registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs);
	std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>> links;
};
std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

#endif // LINK_MAPPING_VERSION_H