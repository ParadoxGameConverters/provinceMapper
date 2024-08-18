#pragma once
#include "LinkMappingVersion.h"

#include <map>
#include <string>

class Automapper
{
  public:
	explicit Automapper(std::shared_ptr<LinkMappingVersion> activeVersion): activeVersion(std::move(activeVersion)) {}
	void registerMatch(const std::shared_ptr<Province>& srcProvince, const std::shared_ptr<Province>& targetProvince);
	void generateLinks();

  private:
	[[nodiscard]] bool isSourceProvinceAvailable(const std::string& srcProvID);
	[[nodiscard]] bool isTargetProvinceAvailable(const std::string& tgtProvID);

	void cleanUpSourceProvinceShares();
	void cleanUpTargetProvinceShares();
	std::set<std::string> srcProvincesToRemove;
	std::set<std::string> tgtProvincesToRemove;

	std::map<std::string, std::map<std::string, int>> sourceProvinceShares; // src prov ID, <target prov ID, shares>
	std::map<std::string, std::map<std::string, int>> targetProvinceShares; // target prov ID, <src prov ID, shares>

	std::set<std::string> srcImpassablesCache;
	std::set<std::string> tgtImpassablesCache;

	std::set<std::string> unavailableSources;
	std::set<std::string> unavailableTargets;

	std::shared_ptr<LinkMappingVersion> activeVersion;
};