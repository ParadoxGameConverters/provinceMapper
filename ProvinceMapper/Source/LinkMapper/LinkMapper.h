#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H
#include "LinkMappingVersion.h"
#include "Parser.h"

class Definitions;
class LinkMapper: commonItems::parser
{
  public:
	void loadMappings(const std::string& linksFileString,
		 const Definitions& sourceDefs,
		 const Definitions& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken);
	void exportMappings(const std::string& linksFile) const;
	[[nodiscard]] const auto& getActiveVersion() const { return activeVersion; }
	[[nodiscard]] const auto& getVersions() const { return versions; }
	void deactivateLink();
	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);

  private:
	void registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs, const std::string& sourceToken, const std::string& targetToken);
	std::vector<std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
};

#endif // LINK_MAPPER_H