#ifndef VIC3_DEFINITIONS_H
#define VIC3_DEFINITIONS_H



#include "DefinitionsInterface.h"



class Vic3Definitions: public DefinitionsInterface
{
	void loadDefinitions() {}

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType) override;

	[[nodiscard]] std::optional<std::string> getNameForChroma(int chroma) override;
	[[nodiscard]] std::optional<std::string> getIDForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForID(const std::string& ID) override;
};



#endif // VIC3_DEFINITIONS_H