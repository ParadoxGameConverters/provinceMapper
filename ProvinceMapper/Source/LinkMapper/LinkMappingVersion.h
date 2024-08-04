#ifndef LINK_MAPPING_VERSION_H
#define LINK_MAPPING_VERSION_H
#include "tpp_interface.hpp"
#include "Definitions/DefinitionsInterface.h"
#include "LinkMapping.h"
#include "TriangulationPointPair.h"
#include "Parser.h"
#include "Triangle.h"

class LinkMappingVersion
{
  public:
	LinkMappingVersion(std::istream& theStream,
		 std::string theVersionName,
		 std::shared_ptr<DefinitionsInterface> theSourceToken,
		 std::shared_ptr<DefinitionsInterface> theTargetToken,
		 std::string sourceToken,
		 std::string targetToken,
		 int theID);

	LinkMappingVersion(std::string theVersionName,
		 std::shared_ptr<DefinitionsInterface> sourceDefs,
		 std::shared_ptr<DefinitionsInterface> targetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);

	[[nodiscard]] const auto& getTriangulationPairs() const { return triangulationPairs; }
	[[nodiscard]] const auto& getLinks() const { return links; }
	[[nodiscard]] const auto& getActiveTriangulationPair() const { return activeTriangulationPair; }
	[[nodiscard]] const auto& getName() const { return versionName; }
	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getUnmappedSources() const { return unmappedSources; }
	[[nodiscard]] const auto& getUnmappedTargets() const { return unmappedTargets; }
	[[nodiscard]] Mapping isProvinceMapped(const std::string& provinceID, bool isSource) const;

	void deactivateLink();
	void deactivateTriangulationPair();
	void activateLinkByIndex(const int row);
	void activateTriangulationPairByIndex(const int row);
	void activateLinkByID(int theID);
	void deleteActiveLink();
	void deleteActiveTriangulationPair();
	void setName(const std::string& theName) { versionName = theName; }
	void setID(int theID) { ID = theID; }
	void copyLinks(const std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>>& theLinks) const { *links = *theLinks; }
	void moveActiveLinkUp() const;
	void moveActiveLinkDown() const;
	void autogenerateMappings(); // TODO: FINISH THIS
	void deleteLinkByID(const int theID);
	void delaunayTriangulate();

	[[nodiscard]] std::optional<int> toggleProvinceByID(const std::string& provinceID, bool isSource);
	[[nodiscard]] int addCommentByIndex(const std::string& comment, int index);
	[[nodiscard]] int addRawLink();
	[[nodiscard]] int addRawComment();
	[[nodiscard]] int addRawTriangulationPair();
	[[nodiscard]] const auto& getTriangles() { return triangles; }


	bool operator==(const LinkMappingVersion& rhs) const;

	friend std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

  private:
	void generateUnmapped() const;
	void removeUnmappedSourceByID(const std::string& provinceID) const;
	void removeUnmappedTargetByID(const std::string& provinceID) const;
	void addUnmappedSourceByID(const std::string& provinceID) const;
	void addUnmappedTargetByID(const std::string& provinceID) const;

	int ID = 0;
	std::string versionName;
	
	int triangulationPairCounter = 0;
	int lastActiveTriangulationPairIndex = 0;
	std::shared_ptr<TriangulationPointPair> activeTriangulationPair;
	std::shared_ptr<std::vector<std::shared_ptr<TriangulationPointPair>>> triangulationPairs;
	std::vector<Triangle> triangles;

	int linkCounter = 0;
	int lastActiveLinkIndex = 0;
	std::shared_ptr<DefinitionsInterface> sourceDefs;
	std::shared_ptr<DefinitionsInterface> targetDefs;
	std::string sourceToken;
	std::string targetToken;

	std::shared_ptr<LinkMapping> activeLink;

	void registerKeys(commonItems::parser& parser);
	std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>> links;
	std::shared_ptr<std::vector<std::shared_ptr<Province>>> unmappedSources;
	std::shared_ptr<std::vector<std::shared_ptr<Province>>> unmappedTargets;
	std::set<std::string> seenSources;
	std::set<std::string> seenTargets;
};
std::ostream& operator<<(std::ostream& output, const LinkMappingVersion& linkMappingVersion);

#endif // LINK_MAPPING_VERSION_H