#ifndef PROVINCE_H
#define PROVINCE_H
#include "Pixel.h"
#include <bitset>
#include <optional>
#include <string>
#include <vector>

class Province final
{
  public:
	enum class ProvinceType: unsigned char
	{
		SeaZones,
		Wasteland,
		ImpassableTerrain,
		Uninhabitable,
		RiverProvinces,
		Lakes,
		ImpassableMountains,
		ImpassableSeas,
		NonOwnable,
		Count
	};

	Province(std::string theID, unsigned char tr, unsigned char tg, unsigned char tb, std::string theName);

	[[nodiscard]] std::string bespokeName() const;
	[[nodiscard]] std::string miscName() const;
	[[nodiscard]] const std::optional<std::string>& getProvinceName() const { return provinceName; }
	[[nodiscard]] const std::optional<std::string>& getAreaName() const { return areaName; }
	[[nodiscard]] const std::optional<std::string>& getRegionName() const { return regionName; }
	[[nodiscard]] const std::optional<std::string>& getSuperRegionName() const { return superRegionName; }
	[[nodiscard]] const std::optional<std::string>& getContinentName() const { return continentName; }
	[[nodiscard]] const std::bitset<static_cast<size_t>(ProvinceType::Count)>& getProvinceTypes() const { return provinceTypes; }
	[[nodiscard]] bool hasProvinceType(ProvinceType type) const { return provinceTypes.test(static_cast<size_t>(type)); }
	[[nodiscard]] bool isWater() const;
	[[nodiscard]] bool isImpassable() const;

	void setProvinceName(std::string name);
	void setAreaName(std::string name);
	void setRegionName(std::string name);
	void setSuperRegionName(std::string name);
	void setContinentName(std::string name);
	void addProvinceType(const std::string& name);

	bool operator==(const Province& rhs) const;
	bool operator==(const Pixel& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;

	std::string ID;
	mutable unsigned char r = 0; // canonical values for color, they may differ from actual pixel colors.
	mutable unsigned char g = 0;
	mutable unsigned char b = 0;
	mutable std::optional<std::string> locName;
	mutable std::string mapDataName;
	std::vector<Pixel> innerPixels; // Not border pixels, just the inner stuff!
	std::vector<Pixel> borderPixels;

  private:
	std::optional<std::string> provinceName; // in case this "province" is actually a /location/
	std::optional<std::string> areaName;
	std::optional<std::string> regionName;
	std::optional<std::string> superRegionName;
	std::optional<std::string> continentName; 
	std::bitset<static_cast<size_t>(ProvinceType::Count)> provinceTypes;
};

#endif // PROVINCE_H
