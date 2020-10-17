#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H
#include "LinkMappingVersion.h"
#include "Parser.h"

class Definitions;
class LinkMapper: commonItems::parser
{
  public:
	void loadMappings(const std::string& linksFileString,
		 const std::shared_ptr<Definitions>& sourceDefs,
		 const std::shared_ptr<Definitions>& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken);
	void exportMappings(const std::string& linksFile) const;
	[[nodiscard]] const auto& getActiveVersion() const { return activeVersion; }
	[[nodiscard]] const auto& getVersions() const { return versions; }
	void deactivateLink() const;
	void activateLinkByIndex(int row) const;
	void activateLinkByID(int ID) const;
	[[nodiscard]] std::optional<int> toggleProvinceByID(int provinceID, bool isSource) const;

  private:
	void registerKeys(const std::shared_ptr<Definitions>& sourceDefs,
		 const std::shared_ptr<Definitions>& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken);
	std::vector<std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
};

#endif // LINK_MAPPER_H