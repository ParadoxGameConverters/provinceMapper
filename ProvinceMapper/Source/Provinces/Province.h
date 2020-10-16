#ifndef PROVINCE_H
#define PROVINCE_H
#include <string>
#include <vector>

struct Pixel;
struct Province
{
	Province(int theID, unsigned char tr, unsigned char tg, unsigned char tb, std::string theName);
	bool operator==(const Province& rhs) const;
	bool operator==(const Pixel& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;

	int ID = 0;
	mutable unsigned char r = 0; // canonical values for color, they may differ from actual pixel colors.
	mutable unsigned char g = 0;
	mutable unsigned char b = 0;
	mutable std::string locName;
	mutable std::string mapDataName;
	std::vector<Pixel> innerPixels; // Not border pixels, just the inner stuff!
	std::vector<Pixel> borderPixels;
};

#endif // PROVINCE_H
