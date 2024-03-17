#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H
#include "Definitions/DefinitionsInterface.h"
#include "LinkMappingVersion.h"
#include "Parser.h"


class LinkMapper: commonItems::parser
{
  public:
	void loadMappings(const std::string& linksFileString,
		 std::shared_ptr<DefinitionsInterface> theSourceDefs,
		 std::shared_ptr<DefinitionsInterface> theTargetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken);

	void exportMappings(const std::string& linksFile) const;
	void deactivateLink() const;
	void activateLinkByIndex(int row) const;
	void activateLinkByID(int ID) const;
	void deleteActiveLink() const;
	void updateActiveVersionName(const std::string& theName) const;
	void moveActiveLinkUp() const;
	void moveActiveLinkDown() const;
	void moveActiveVersionLeft();
	void moveActiveVersionRight();

	[[nodiscard]] const auto& getActiveVersion() const { return activeVersion; }
	[[nodiscard]] const auto& getVersions() const { return versions; }
	[[nodiscard]] std::optional<int> toggleProvinceByID(const std::string& provinceID, bool isSource) const;
	[[nodiscard]] std::optional<int> addCommentByIndex(const std::string& comment, int index) const;
	[[nodiscard]] std::optional<int> addRawLink() const;
	[[nodiscard]] std::optional<int> addRawComment() const;
	[[nodiscard]] std::optional<int> addRawTriangulationPair() const;
	[[nodiscard]] const std::shared_ptr<LinkMappingVersion>& addVersion();
	[[nodiscard]] const std::shared_ptr<LinkMappingVersion>& copyVersion();
	[[nodiscard]] const std::shared_ptr<LinkMappingVersion>& deleteVersion();
	[[nodiscard]] const std::shared_ptr<LinkMappingVersion>& activateVersionByIndex(int index);
	[[nodiscard]] Mapping isProvinceMapped(const std::string& provinceID, bool isSource) const;

  private:
	void registerKeys();

	std::shared_ptr<DefinitionsInterface> sourceDefs;
	std::shared_ptr<DefinitionsInterface> targetDefs;
	std::string sourceToken;
	std::string targetToken;
	std::vector<std::shared_ptr<LinkMappingVersion>> versions;
	std::shared_ptr<LinkMappingVersion> activeVersion;

	int versionCounter = 0; // This is an internal counter that is used when assigning new version names and IDs.
};

#endif // LINK_MAPPER_H