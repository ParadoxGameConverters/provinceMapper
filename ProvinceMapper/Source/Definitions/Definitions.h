#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include "DefinitionsInterface.h"
#include "EU4Regions/RegionManager.h"

struct Province;
class Definitions: public DefinitionsInterface
{
  public:
	void loadDefinitions(const std::string& fileName, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);
	void loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType) override;
	void loadVic3Regions(const std::string& folderPath) override {}

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;

	[[nodiscard]] std::optional<std::string> getNameForChroma(int chroma) override;
	[[nodiscard]] std::optional<std::string> getMiscForChroma(int chroma) override;
	[[nodiscard]] std::optional<std::string> getIDForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForID(const std::string& ID) override;

  private:
	void parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);

	EU4::RegionManager eu4RegionManager;
};

#endif // DEFINITIONS_H
