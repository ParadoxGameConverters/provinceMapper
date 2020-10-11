#include "Province.h"

Pixel::Pixel(int tx, int ty, unsigned char tr, unsigned char tg, unsigned char tb): x(tx), y(tx), r(tr), g(tg), b(tb)
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

bool Province::operator!() const
{
	return !ID;
}
