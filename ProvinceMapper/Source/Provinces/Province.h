#ifndef PROVINCE_H
#define PROVINCE_H
#include <string>
#include <vector>

struct Pixel
{
	Pixel() = default;
	explicit Pixel(int tx, int ty, unsigned char tr, unsigned char tg, unsigned char tb);
	int x = 0;
	int y = 0;
	unsigned char r = 0; // these colors vary depending on the state of the province.
	unsigned char g = 0;
	unsigned char b = 0;
};

struct Province
{
	bool operator==(const Province& rhs) const;
	bool operator==(const Pixel& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;
	bool operator!() const;

	int ID = 0;
	mutable unsigned char r = 0; // canonical values for color, they may differ from actual pixel colors.
	mutable unsigned char g = 0;
	mutable unsigned char b = 0;
	mutable std::string locName;
	mutable std::string mapDataName;
	std::vector<Pixel> pixels;
};

#endif // PROVINCE_H
