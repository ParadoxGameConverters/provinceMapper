#ifndef PROVINCE_H
#define PROVINCE_H
#include "Pixel.h"
#include <optional>
#include <string>
#include <vector>

struct Province
{
	Province(std::string theID, unsigned char tr, unsigned char tg, unsigned char tb, std::string theName);

	[[nodiscard]] std::string bespokeName() const;
	[[nodiscard]] std::string miscName() const;
	[[nodiscard]] bool isWater() const;
	[[nodiscard]] bool isImpassable() const;

	bool operator==(const Province& rhs) const;
	bool operator==(const Pixel& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;

	std::string ID;
	mutable unsigned char r = 0; // canonical values for color, they may differ from actual pixel colors.
	mutable unsigned char g = 0;
	mutable unsigned char b = 0;
	mutable std::optional<std::string> locName;
	mutable std::string mapDataName;
	std::vector<Pixel> innerPixels; // Not border pixels, just the inner stuff!
	std::vector<Pixel> borderPixels;
	std::optional<std::string> areaName;
	std::optional<std::string> regionName;
	std::optional<std::string> superRegionName;
	std::optional<std::string> provinceType;
};

#endif // PROVINCE_H
