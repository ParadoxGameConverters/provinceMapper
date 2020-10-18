#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H
#include "LinkMappingVersion.h"
#include "Parser.h"

class Definitions;
class LinkMapper: commonItems::parser
{
  public:
	void loadMappings(const std::string& linksFileString,
		 std::shared_ptr<Definitions> theSourceDefs,
		 std::shared_ptr<Definitions> theTargetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken);
	
	void exportMappings(const std::string& linksFile) const;
	void deactivateLink() const;
	void activateLinkByIndex(int row) const;
	void activateLinkByID(int ID) const;

	[[nodiscard]] const auto& getActiveVersion() const { return activeVersion; }
	[[nodiscard]] const auto& getVersions() const { return versions; }
	[[nodiscard]] std::optional<int> toggleProvinceByID(int provinceID, bool isSource) const;

  private:
	void registerKeys();

	std::shared_ptr<Definitions> sourceDefs;
	std::shared_ptr<Definitions> targetDefs;
	std::string sourceToken;
	std::string targetToken;
	std::vector<std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;
};

#endif // LINK_MAPPER_H