#ifndef YML_SCRAPER
#define YML_SCRAPER
#include <map>
#include <string>

class YmlScraper
{
  public:
	YmlScraper(const std::string& fileName);

	[[nodiscard]] const auto& getLocalizations() const { return localizations; }

  private:
	void scrapeStream(std::istream& theStream);

	std::map<std::string, std::string> localizations;
};

#endif // YML_SCRAPER