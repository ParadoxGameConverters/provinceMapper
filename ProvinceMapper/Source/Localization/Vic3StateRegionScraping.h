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

  private:
	void registerKeys();

	std::set<std::string> provinces;
};

#endif // VIC3_STATE_REGION_SCRAPING