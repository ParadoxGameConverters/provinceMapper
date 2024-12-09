#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "robin_hood.h"
#include <future>
#include <map>
#include <mutex>
#include <ranges>
#include <set>
#include <string>

wxPoint triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint);


class LinkMappingVersion;

struct wxPointHash
{
	std::size_t operator()(const wxPoint& point) const { return std::hash<int>()(point.x) ^ (std::hash<int>()(point.y) << 1); }
};

struct wxPointEqual
{
	bool operator()(const wxPoint& a, const wxPoint& b) const { return a.x == b.x && a.y == b.y; }
};

class Triangle;
class Province;
typedef robin_hood::unordered_map<wxPoint, std::shared_ptr<Triangle>, wxPointHash, wxPointEqual> PointToTriangleMap;
typedef robin_hood::unordered_map<wxPoint, std::shared_ptr<Province>, wxPointHash, wxPointEqual> PointToProvinceMap;


struct Pixel;
class wxTaskBarButton;
class Automapper final
{
  public:
	explicit Automapper(std::shared_ptr<LinkMappingVersion> activeVersion): activeVersion(std::move(activeVersion)) {}
	
	void matchTargetProvsToSourceProvs(const std::vector<std::shared_ptr<Province>>& sourceProvinces,
		 const PointToTriangleMap& srcPointToTriangleMap,
		 const PointToProvinceMap& tgtPointToProvinceMap,
		 int targetMapWidth,
		 int targetMapHeight);
	void registerMatch(const std::shared_ptr<Province>& srcProvince, const std::shared_ptr<Province>& targetProvince);
	void generateLinks(wxTaskBarButton* taskBarBtn);

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

	std::set<std::string> srcImpassablesCache;
	std::set<std::string> tgtImpassablesCache;

	std::set<std::string> alreadyMappedSrcProvincesCache;
	std::set<std::string> alreadyMappedTgtProvincesCache;

	std::shared_ptr<LinkMappingVersion> activeVersion;

	std::mutex automapperMutex; // Mutex for thread safety

	inline void determineTargetProvinceForSourcePixels(const std::shared_ptr<Province>& sourceProvince,
		 const std::vector<Pixel>& sourcePixels,
		 const PointToTriangleMap& srcPointToTriangleMap,
		 const PointToProvinceMap& tgtPointToProvinceMap,
		 int targetMapWidth,
		 int targetMapHeight);
};