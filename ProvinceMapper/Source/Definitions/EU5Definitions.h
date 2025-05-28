#ifndef EU5_DEFINITIONS_H
#define EU5_DEFINITIONS_H
#include "DefinitionsInterface.h"
#include "EU5Regions/EU5RegionManager.h"

class Province;
class EU5Definitions: public DefinitionsInterface
{
  public:
	void loadDefinitions(const std::filesystem::path& fileName, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);
	void loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType) override;
	void loadVic3Regions(const std::filesystem::path& folderPath) override {}

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerNeighbor(unsigned int provinceChroma, unsigned int neighborChroma) override;
	void ditchAdjacencies(const std::filesystem::path& fileName) override;

	[[nodiscard]] std::optional<std::string> getNameForChroma(unsigned int chroma) override;
	[[nodiscard]] std::optional<std::string> getMiscForChroma(unsigned int chroma) override;
	[[nodiscard]] std::optional<std::string> getIDForChroma(unsigned int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForChroma(unsigned int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForID(const std::string& ID) override;
	[[nodiscard]] std::map<unsigned int, std::set<unsigned int>> getNeighborChromas() const override;

  private:
	void registerKeys();
	void filloutProvinceDetails(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);
	void tryToLoadProvinceTypes(const std::filesystem::path& mapDataPath);

	commonItems::parser defParser;

	EU5::EU5RegionManager eu5RegionManager;
	std::map<unsigned int, std::set<unsigned int>> neighborChromas; // chroma-> neighbor chromas
};

#endif // DEFINITIONS_H
