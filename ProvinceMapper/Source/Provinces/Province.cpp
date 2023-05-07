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

std::string Province::getLowerCaseProvinceType() const
{
	std::string provinceTypeLowerCase = provinceType;
	std::transform(provinceTypeLowerCase.begin(), provinceTypeLowerCase.end(), provinceTypeLowerCase.begin(), ::tolower);
	return provinceTypeLowerCase;
}

bool Province::isWater() const
{	
	auto provinceTypeLowerCase = getLowerCaseProvinceType();
	return provinceTypeLowerCase == "sea_zones" || provinceTypeLowerCase == "river_provinces" || provinceTypeLowerCase == "lakes" || provinceTypeLowerCase == "impassable_seas";
}

bool Province::isImpassable() const
{
	auto provinceTypeLowerCase = getLowerCaseProvinceType();
	return provinceTypeLowerCase == "wasteland" || provinceTypeLowerCase == "impassable_terrain" || provinceTypeLowerCase == "impassable_mountains";
}