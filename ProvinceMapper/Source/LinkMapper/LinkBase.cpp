#include "LinkBase.h"

LinkBase::LinkBase(int theID): ID(theID)
{
}


bool LinkBase::operator==(const LinkBase& rhs) const
{
	return ID == rhs.ID;
}
