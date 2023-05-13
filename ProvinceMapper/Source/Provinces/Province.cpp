#include "Province.h"
#include <algorithm>

Province::Province(std::string theID, const unsigned char tr, const unsigned char tg, const unsigned char tb, std::string theName):
	 ID(std::move(theID)), r(tr), g(tg), b(tb), mapDataName(std::move(theName))
{
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
	name = "\nType: " + provinceType;
	if (areaName)
		name += "\nArea: " + *areaName;
	if (regionName)
		name += "\nRegion: " + *regionName;
	if (superRegionName)
		name += "\nSuperRegion: " + *superRegionName;
	return name;
}

bool Province::isWater() const
{	
	// For EU4, use region and area names to determine what is water.
	if (superRegionName && superRegionName.value().ends_with("sea_superregion"))
	{
		return true;
	}
	if (areaName && areaName.value().ends_with("sea_area"))
	{
		return true;
	}
	// For games like I:R and CK3, province type is defined and can be used.
	return provinceType == "sea_zones" || provinceType == "river_provinces" || provinceType == "lakes" || provinceType == "impassable_seas";
}

bool Province::isImpassable() const
{
	return provinceType == "wasteland" || provinceType == "impassable_terrain" || provinceType == "impassable_mountains";
}