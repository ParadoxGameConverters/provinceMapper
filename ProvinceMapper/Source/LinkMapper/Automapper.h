#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <gtl/phmap.hpp>
#include <future>
#include <map>
#include <mutex>
#include <string>

wxPoint triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint);


class LinkMappingVersion;

struct wxPointHash
{
	std::size_t operator()(const wxPoint& point) const { return std::hash<int>()(point.x) ^ (std::hash<int>()(point.y) << 1); }
};

class Triangle;
class Province;
typedef gtl::flat_hash_map<wxPoint, std::shared_ptr<Triangle>, wxPointHash> PointToTriangleMap;
typedef gtl::flat_hash_map<wxPoint, std::shared_ptr<Province>, wxPointHash> PointToProvinceMap;


struct Pixel;
class wxTaskBarButton;
class Automapper final
{
  public:
	explicit Automapper(std::shared_ptr<LinkMappingVersion> activeVersion): activeVersion(std::move(activeVersion)) {}
	
	void matchTargetProvsToSourceProvs(const std::vector<std::shared_ptr<Province>>& sourceProvinces,
		 const PointToTriangleMap& srcPointToTriangleMap,
		 const PointToProvinceMap& tgtPointToLandProvinceMap,
		 const PointToProvinceMap& tgtPointToWaterProvinceMap,
		 const gtl::flat_hash_set<std::string>& excludedTgtProvinceIDs,
		 int targetMapWidth,
		 int targetMapHeight);
	void registerMatch(const std::shared_ptr<Province>& srcProvince, const std::shared_ptr<Province>& targetProvince, int amount);
	void generateLinks(wxTaskBarButton* taskBarBtn, bool mapSourceImpassables, bool mapTargetImpassables);

  private:
	[[nodiscard]] bool canProvincesBeMapped(const std::string& srcProvID, const std::string& tgtProvID, bool allowAddingToExistingLink) const;
	void mapProvinces(const std::string& srcProvID, const std::string& tgtProvID);

	void forUnmappedTargetsMapUnmappedSources();
	void forUnmappedSourcesMapUnmappedTargets();
	void forNonImpassableTargetsMapTryToMapFirstChoices();
	void forNonImpassableSourcesMapTryToMapFirstChoices();
	void forUnmappedTargetsEvaluateAllNonImpassableMatches();
	void forUnmappedSourcesEvaluateAllNonImpassableMatches();
	void forUnmappedTargetsEvaluateAllMatches();
	void forUnmappedSourcesEvaluateAllMatches();
	void forUnmappedTargetsTryToStealSourcesFromExistingLinks();
	void forUnmappedSourcesTryToStealTargetsFromExistingLinks();

	void cleanUpSourceProvinceShares();
	void cleanUpTargetProvinceShares();

	std::map<std::string, std::map<std::string, int>> sourceProvinceShares; // src prov ID, <target prov ID, shares>
	std::map<std::string, std::map<std::string, int>> targetProvinceShares; // target prov ID, <src prov ID, shares>

	gtl::flat_hash_set<std::string> srcImpassablesCache;
	gtl::flat_hash_set<std::string> tgtImpassablesCache;

	gtl::flat_hash_set<std::string> alreadyMappedSrcProvincesCache;
	gtl::flat_hash_set<std::string> alreadyMappedTgtProvincesCache;

	std::shared_ptr<LinkMappingVersion> activeVersion;

	std::mutex automapperMutex; // Mutex for thread safety

	inline void determineTargetProvinceForSourcePixels(const std::shared_ptr<Province>& sourceProvince,
		 const std::vector<Pixel>& sourcePixels,
		 const PointToTriangleMap& srcPointToTriangleMap,
		 const PointToProvinceMap& tgtPointToProvinceMap,
		 const gtl::flat_hash_set<std::string>& excludedTgtProvinceIDs,
		 int targetMapWidth,
		 int targetMapHeight);
};