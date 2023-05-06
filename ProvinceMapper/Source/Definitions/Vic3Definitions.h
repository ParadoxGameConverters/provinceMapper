#ifndef VIC3_DEFINITIONS_H
#define VIC3_DEFINITIONS_H
#include "DefinitionsInterface.h"
#include "Vic3Regions/SuperRegionLoader.h"


class Vic3Definitions: public DefinitionsInterface
{
	void loadDefinitions() {}

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType) override;
	void loadVic3Regions(const std::string& folderPath) override;
	void registerNeighbor(unsigned int provinceChroma, unsigned int neighborChroma) override;
	void ditchAdjacencies(const std::string& fileName) override;

	[[nodiscard]] std::optional<std::string> getNameForChroma(unsigned int chroma) override;
	[[nodiscard]] std::optional<std::string> getMiscForChroma(unsigned int chroma) override;
	[[nodiscard]] std::optional<std::string> getIDForChroma(unsigned int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForChroma(unsigned int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForID(const std::string& ID) override;
	[[nodiscard]] std::map<unsigned int, std::set<unsigned int>> getNeighborChromas() const override;

	V3::SuperRegionLoader vic3regions;
	std::map<unsigned int, std::set<unsigned int>> neighborChromas; // chroma-> neighbor chromas
};

#endif // VIC3_DEFINITIONS_H