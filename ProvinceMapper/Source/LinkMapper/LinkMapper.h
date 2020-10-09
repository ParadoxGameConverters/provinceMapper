#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H

#include "LinkMappingVersion.h"
#include "Parser.h"

class LinkMapper: commonItems::parser
{
  public:
	LinkMapper();
	explicit LinkMapper(std::istream& theStream);

	void exportMappings() const;
	[[nodiscard]] const auto& getLinks() const { return activeVersion->getLinks(); }

  private:
	void registerKeys();
	std::map<std::string, std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
};

#endif // LINK_MAPPER_H