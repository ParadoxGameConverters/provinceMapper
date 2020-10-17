#ifndef LINK_MAPPING_VERSION_H
#define LINK_MAPPING_VERSION_H
#include "LinkMapping.h"
#include "Parser.h"

class Definitions;
class LinkMappingVersion: commonItems::parser
{
  public:
	LinkMappingVersion(std::istream& theStream,
		 std::string theVersionName,
		 const std::shared_ptr<Definitions>& sourceDefs,
		 const std::shared_ptr<Definitions>& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken);

	[[nodiscard]] const auto& getLinks() const { return links; }
	[[nodiscard]] const auto& getName() const { return versionName; }

	void deactivateLink();
	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void toggleProvinceByID(int provinceID, bool isSource) const;

	friend std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

  private:
	std::string versionName;
	int linkCounter = 0;

	std::shared_ptr<LinkMapping> activeLink;

	void registerKeys(const std::shared_ptr<Definitions>& sourceDefs,
		 const std::shared_ptr<Definitions>& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken);
	std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>> links;
};
std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

#endif // LINK_MAPPING_VERSION_H