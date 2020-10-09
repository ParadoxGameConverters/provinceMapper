#include "Province.h"

Province::Province(int theID): ID(theID)
{
}

bool Province::operator==(const Province& rhs) const
{
	return ID == rhs.ID;
}

bool Province::operator!=(const Province& rhs) const
{
	return ID != rhs.ID;
}

bool Province::operator<(const Province& rhs) const
{	
	return ID < rhs.ID;
}

bool Province::operator!() const
{
	return !ID;
}
