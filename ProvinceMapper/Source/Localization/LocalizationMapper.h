#ifndef LOCALIZATION_MAPPER
#define LOCALIZATION_MAPPER
#include <map>
#include <optional>
#include <string>

class LocalizationMapper
{
  public:
	enum class LocType
	{
		SOURCE,
		TARGET
	};

	void scrapeSourceDir(const std::string& dirPath);
	void scrapeTargetDir(const std::string& dirPath);

	[[nodiscard]] std::optional<std::string> getLocForSourceKey(const std::string& key) const;
	[[nodiscard]] std::optional<std::string> getLocForTargetKey(const std::string& key) const;

  private:
	void scrapeFile(const std::string& filePath, LocType locType);
	std::map<std::string, std::string> sourceLocalizations;
	std::map<std::string, std::string> targetLocalizations;
};

#endif // LOCALIZATION_MAPPER