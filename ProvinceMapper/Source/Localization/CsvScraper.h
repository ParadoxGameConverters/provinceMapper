#ifndef CSV_SCRAPER
#define CSV_SCRAPER
#include <filesystem>
#include <map>
#include <string>

class CsvScraper
{
  public:
	CsvScraper(const std::filesystem::path& fileName);

	[[nodiscard]] const auto& getLocalizations() const { return localizations; }

  private:
	void scrapeStream(std::istream& theStream);

	std::map<std::string, std::string> localizations; // key, value
};

#endif // CSV_SCRAPER