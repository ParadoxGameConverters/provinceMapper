#include "Province.h"
#include <Configuration/Configuration.h>
#include <algorithm>
#include <numeric>

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

void Province::addProvinceType(std::string name)
{
	provinceTypes.emplace(std::move(name));
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
	if (!provinceTypes.empty())
	{
		name += std::accumulate(++provinceTypes.begin(), provinceTypes.end(), *provinceTypes.begin(), [](const std::string& a, const std::string& b) {
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

	// For games like I:R and CK3, province type is defined and can be used.
	return provinceTypes.contains("sea_zones") || provinceTypes.contains("river_provinces") || provinceTypes.contains("lakes") ||
			 provinceTypes.contains("impassable_seas");
}

bool Province::isImpassable() const
{
	return provinceTypes.contains("wasteland") || provinceTypes.contains("impassable_terrain") || provinceTypes.contains("impassable_mountains");
}