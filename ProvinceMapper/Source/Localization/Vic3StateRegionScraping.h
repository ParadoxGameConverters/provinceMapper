#ifndef VIC3_STATE_REGION_SCRAPING
#define VIC3_STATE_REGION_SCRAPING
#include <Parser.h>
#include <set>
#include <string>

class Vic3StateRegionScraping: commonItems::parser
{
  public:
	Vic3StateRegionScraping() = default;
	Vic3StateRegionScraping(std::istream& theStream);

	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] bool isProvinceMine(const std::string& province) const { return mines.contains(province); }
	[[nodiscard]] bool isProvinceFarm(const std::string& province) const { return farms.contains(province); }
	[[nodiscard]] bool isProvinceWood(const std::string& province) const { return woods.contains(province); }
	[[nodiscard]] bool isProvinceCity(const std::string& province) const { return cities.contains(province); }
	[[nodiscard]] bool isProvincePort(const std::string& province) const { return ports.contains(province); }

  private:
	void registerKeys();

	std::set<std::string> provinces;
	std::set<std::string> mines;
	std::set<std::string> farms;
	std::set<std::string> ports;
	std::set<std::string> woods;
	std::set<std::string> cities;
};

#endif // VIC3_STATE_REGION_SCRAPING