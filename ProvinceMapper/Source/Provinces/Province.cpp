#include "Province.h"

#include "Log.h"

#include <Configuration/Configuration.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <numeric>
#include <string_view>

namespace
{
	using ProvinceType = Province::ProvinceType;

	constexpr std::array<std::string_view, static_cast<size_t>(ProvinceType::Count)> ProvinceTypeNames = {
		"sea_zones",
		"wasteland",
		"impassable_terrain",
		"uninhabitable",
		"river_provinces",
		"lakes",
		"impassable_mountains",
		"impassable_seas",
		"non_ownable"
	};

	const std::bitset<static_cast<size_t>(ProvinceType::Count)> WaterTypesMask = [] {
		std::bitset<static_cast<size_t>(ProvinceType::Count)> mask;
		mask.set(static_cast<size_t>(ProvinceType::SeaZones));
		mask.set(static_cast<size_t>(ProvinceType::RiverProvinces));
		mask.set(static_cast<size_t>(ProvinceType::Lakes));
		mask.set(static_cast<size_t>(ProvinceType::ImpassableSeas));
		return mask;
	}();

	// NonOwnable is included because it is a wasteland type - these provinces cannot be owned or settled.
	// All types in this mask represent provinces that should be excluded from automapping.
	const std::bitset<static_cast<size_t>(ProvinceType::Count)> ImpassableOrWastelandTypesMask = [] {
		std::bitset<static_cast<size_t>(ProvinceType::Count)> mask;
		mask.set(static_cast<size_t>(ProvinceType::Wasteland));
		mask.set(static_cast<size_t>(ProvinceType::ImpassableTerrain));
		mask.set(static_cast<size_t>(ProvinceType::ImpassableMountains));
		mask.set(static_cast<size_t>(ProvinceType::NonOwnable));
		return mask;
	}();

	ProvinceType toProvinceType(const std::string& name)
	{
		for (size_t i = 0; i < ProvinceTypeNames.size(); ++i)
		{
			if (name == ProvinceTypeNames[i])
				return static_cast<ProvinceType>(i);
		}

		const std::string errMsg = "Unsupported province type name: " + name;
		Log(LogLevel::Error) << errMsg;
		throw std::invalid_argument(errMsg);
	}
}

Province::Province(std::string theID, const unsigned char tr, const unsigned char tg, const unsigned char tb, std::string theName):
	 ID(std::move(theID)), r(tr), g(tg), b(tb), mapDataName(std::move(theName))
{
}

void Province::setProvinceName(std::string name)
{
	provinceName = std::move(name);
}

void Province::setAreaName(std::string name)
{
	areaName = std::move(name);
}

void Province::setRegionName(std::string name)
{
	regionName = std::move(name);
}

void Province::setSuperRegionName(std::string name)
{
	superRegionName = std::move(name);
}

void Province::setContinentName(std::string name)
{
	continentName = std::move(name);
}

void Province::addProvinceType(const std::string& name)
{
	provinceTypes.set(static_cast<size_t>(toProvinceType(name)));
}

bool Province::operator==(const Province& rhs) const
{
	return ID == rhs.ID;
}

bool Province::operator==(const Pixel& rhs) const
{
	return r == rhs.r && g == rhs.g && b == rhs.b;
}

bool Province::operator!=(const Province& rhs) const
{
	return ID != rhs.ID;
}

bool Province::operator<(const Province& rhs) const
{
	return ID < rhs.ID;
}

std::string Province::bespokeName() const
{
	std::string name;
	if (locName)
	{
		name = *locName;
		if (!mapDataName.empty())
			name += " (" + mapDataName + ")";
	}
	else if (!mapDataName.empty())
		name = mapDataName;
	else
		name = "(Unknown)";
	return name;
}

std::string Province::miscName() const
{
	std::string name;

	name = "\nTypes: ";
	std::vector<std::string> types;
	for (size_t i = 0; i < ProvinceTypeNames.size(); ++i)
	{
		if (provinceTypes.test(i))
			types.emplace_back(ProvinceTypeNames[i]);
	}
	if (!types.empty())
	{
		name += std::accumulate(++types.begin(), types.end(), *types.begin(), [](const std::string& a, const std::string& b) {
			return a + ", " + b;
		});
	}
	else
	{
		name += "normal";
	}

	if (provinceName)
		name += "\nProvince: " + *provinceName;
	if (areaName)
		name += "\nArea: " + *areaName;
	if (regionName)
		name += "\nRegion: " + *regionName;
	if (superRegionName)
		name += "\nSuperRegion: " + *superRegionName;
	if (continentName)
		name += "\nContinent: " + *continentName;
	return name;
}

bool Province::isWater() const
{
	// For games like I:R and CK3, province type is defined and can be used.
	if ((provinceTypes & WaterTypesMask).any()) // cheap bitset operation
	{
		return true;
	}

	// For EU4, use region and area names to determine what is water.
	if (superRegionName && superRegionName.value().ends_with("_sea_superregion"))
	{
		return true;
	}
	if (regionName && regionName.value().ends_with("_sea_region"))
	{
		return true;
	}
	if (areaName && areaName.value().ends_with("_sea_area"))
	{
		return true;
	}

	return false;
}

bool Province::isImpassableOrWasteland() const
{
	return (provinceTypes & ImpassableOrWastelandTypesMask).any();
}