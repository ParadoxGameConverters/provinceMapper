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
		 std::shared_ptr<Definitions> theSourceToken,
		 std::shared_ptr<Definitions> theTargetToken,
		 std::string sourceToken,
		 std::string targetToken);

	LinkMappingVersion(std::string theVersionName,
		 std::shared_ptr<Definitions> sourceDefs,
		 std::shared_ptr<Definitions> targetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken);

	[[nodiscard]] const auto& getLinks() const { return links; }
	[[nodiscard]] const auto& getName() const { return versionName; }

	void deactivateLink();
	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	[[nodiscard]] std::optional<int> toggleProvinceByID(int provinceID, bool isSource);
	[[nodiscard]] int addCommentByIndex(const std::string& comment, int index);

	friend std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

  private:
	std::string versionName;
	int linkCounter = 0;
	int lastActiveLinkIndex = 0;
	std::shared_ptr<Definitions> sourceDefs;
	std::shared_ptr<Definitions> targetDefs;
	std::string sourceToken;
	std::string targetToken;

	std::shared_ptr<LinkMapping> activeLink;

	void registerKeys();
	std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>> links;
};
std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

#endif // LINK_MAPPING_VERSION_H