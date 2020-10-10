#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H

#include "LinkMappingVersion.h"
#include "Parser.h"

class Definitions;
class LinkMapper: commonItems::parser
{
  public:
	void loadMappings(const std::string& fileName, const Definitions& sourceDefs, const Definitions& targetDefs);
	void exportMappings() const;
	[[nodiscard]] const auto& getLinks() const { return activeVersion->getLinks(); }

  private:
	void registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs);
	std::map<std::string, std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
};

#endif // LINK_MAPPER_H