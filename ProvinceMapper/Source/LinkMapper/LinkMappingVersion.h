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
		 std::string targetToken,
		 int theID);

	LinkMappingVersion(std::string theVersionName,
		 std::shared_ptr<Definitions> sourceDefs,
		 std::shared_ptr<Definitions> targetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);

	[[nodiscard]] const auto& getLinks() const { return links; }
	[[nodiscard]] const auto& getName() const { return versionName; }
	[[nodiscard]] auto getID() const { return ID; }

	void deactivateLink();
	void activateLinkByIndex(int row);
	void activateLinkByID(int theID);
	void deleteActiveLink();
	void setName(const std::string& theName) { versionName = theName; }
	void setID(int theID) { ID = theID; }
	void copyLinks(const std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>>& theLinks) const { *links = *theLinks; }
	void moveActiveLinkUp() const;
	void moveActiveLinkDown() const;

	[[nodiscard]] std::optional<int> toggleProvinceByID(int provinceID, bool isSource);
	[[nodiscard]] int addCommentByIndex(const std::string& comment, int index);
	[[nodiscard]] int addRawLink();
	[[nodiscard]] int addRawComment();

	friend std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

  private:
	int ID = 0;
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