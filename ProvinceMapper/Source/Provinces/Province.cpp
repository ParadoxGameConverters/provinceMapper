#include "Province.h"

Province::Province(const int theID, const unsigned char tr, const unsigned char tg, const unsigned char tb, std::string theName):
	 ID(theID), r(tr), g(tg), b(tb), mapDataName(std::move(theName))
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
