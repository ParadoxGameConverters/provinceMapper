#include "Pixel.h"

Pixel::Pixel(const int tx, const int ty, const unsigned char tr, const unsigned char tg, const unsigned char tb): x(tx), y(ty), r(tr), g(tg), b(tb)
{
}

unsigned int pixelPack(const unsigned char r, const unsigned char g, const unsigned char b)
{
	return r << 16 | g << 8 | b;
}

int coordsToOffset(const int x, const int y, const int width)
{
	return (y * width + x) * 3;
}
