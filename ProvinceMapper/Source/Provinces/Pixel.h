#ifndef PIXEL_H
#define PIXEL_H

struct Pixel
{
	Pixel() = default;
	explicit Pixel(int tx, int ty, unsigned char tr, unsigned char tg, unsigned char tb);
	int x = 0;
	int y = 0;
	// these colors mirror province color and are used when we're mangling oodles of pixels, as opposed to provinces themselves.
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
};

unsigned int pixelPack(unsigned char r, unsigned char g, unsigned char b);
int coordsToOffset(int x, int y, int width);

#endif // PIXEL_H
