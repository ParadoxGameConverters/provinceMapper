#ifndef DEFINITIONS_H
#define DEFINITIONS_H



#include "DefinitionsInterface.h"



struct Province;
class Definitions: public DefinitionsInterface
{
  public:
	void loadDefinitions(const std::string& fileName, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) override;

	[[nodiscard]] std::optional<std::string> getNameForChroma(int chroma) override;
	[[nodiscard]] std::optional<std::string> getIDForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForChroma(int chroma) override;
	[[nodiscard]] std::shared_ptr<Province> getProvinceForID(const std::string& ID) override;

  private:
	void parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);
};

#endif // DEFINITIONS_H
