#ifndef DEFINITIONS_INTERFACE_H
#define DEFINITIONS_INTERFACE_H
#include "Localization/LocalizationMapper.h"
#include <map>
#include <memory>
#include <optional>
#include <string>


struct Province;


class DefinitionsInterface
{
  public:
	virtual void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType) = 0;
	virtual void loadVic3Regions(const std::string& folderPath) = 0;

	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] virtual std::optional<std::string> getNameForChroma(int chroma) = 0;
	[[nodiscard]] virtual std::optional<std::string> getMiscForChroma(int chroma) = 0;
	[[nodiscard]] virtual std::optional<std::string> getIDForChroma(int chroma) = 0;
	[[nodiscard]] virtual std::shared_ptr<Province> getProvinceForChroma(int chroma) = 0;
	[[nodiscard]] virtual std::shared_ptr<Province> getProvinceForID(const std::string& ID) = 0;

  protected:
	std::map<std::string, std::shared_ptr<Province>> provinces;		// ID, province
	std::map<unsigned int, std::shared_ptr<Province>> chromaCache; // color, province
};



#endif // DEFINITIONS_INTERFACE_H