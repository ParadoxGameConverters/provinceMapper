#ifndef PROVINCE_H
#define PROVINCE_H
#include <string>
#include <vector>

struct Pixel
{
	int x = 0;
	int y = 0;
	int r = 0; // these colors vary depending on the state of the province.
	int g = 0;
	int b = 0;
};

struct Province
{
	bool operator==(const Province& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;
	bool operator!() const;

	int ID = 0;
	mutable int r = 0; // canonical values for color, they may differ from actual pixel colors.
	mutable int g = 0;
	mutable int b = 0;
	mutable std::string locName;
	mutable std::string mapDataName;
	std::vector<Pixel> pixels;
};

#endif // PROVINCE_H
