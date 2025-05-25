#ifndef VIC3_STATE_REGION_SCRAPER
#define VIC3_STATE_REGION_SCRAPER
#include <Parser.h>
#include <map>
#include <string>

class Vic3StateRegionScraper: commonItems::parser
{
  public:
	Vic3StateRegionScraper(const std::filesystem::path& fileName);

	[[nodiscard]] const auto& getLocalizations() const { return localizations; }

  private:
	void registerKeys();

	std::map<std::string, std::string> localizations; // in this case <x123456, STATE_EXAMPLARIA>
};

#endif // VIC3_STATE_REGION_SCRAPER