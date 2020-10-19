#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include "Localization/LocalizationMapper.h"
#include <map>
#include <memory>
#include <optional>
#include <string>

struct Province;
class Definitions
{
  public:
	void loadDefinitions(const std::string& fileName, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

	[[nodiscard]] const auto& getProvinces() const { return provinces; }

	std::optional<std::string> getNameForChroma(int chroma);
	std::shared_ptr<Province> getProvinceForChroma(int chroma);
	std::shared_ptr<Province> getProvinceForID(int ID);

  private:
	void parseStream(std::istream& theStream, const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType);
	[[nodiscard]] std::optional<std::tuple<int, unsigned char, unsigned char, unsigned char, std::string>> parseLine(const std::string& line);

	std::map<int, std::shared_ptr<Province>> provinces;				// ID, province
	std::map<unsigned int, std::shared_ptr<Province>> chromaCache; // color, province
};

#endif // DEFINITIONS_H
