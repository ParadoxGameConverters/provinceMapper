#include "Province.h"

Pixel::Pixel(const int tx, const int ty, const unsigned char tr, const unsigned char tg, const unsigned char tb): x(tx), y(ty), r(tr), g(tg), b(tb)
{
}

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

unsigned int pixelPack(unsigned char r, unsigned char g, unsigned char b)
{
	return r << 16 | g << 8 | b;
}
