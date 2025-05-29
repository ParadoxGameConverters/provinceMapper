#ifndef LOCALIZATION_MAPPER
#define LOCALIZATION_MAPPER
#include <filesystem>
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

	void scrapeSourceDir(const std::filesystem::path& dirPath);
	void scrapeTargetDir(const std::filesystem::path& dirPath);

	[[nodiscard]] std::optional<std::string> getLocForSourceKey(const std::string& key) const;
	[[nodiscard]] std::optional<std::string> getLocForTargetKey(const std::string& key) const;

  private:
	void scrapeFile(const std::filesystem::path& filePath, LocType locType);
	std::map<std::string, std::string> sourceLocalizations;
	std::map<std::string, std::string> targetLocalizations;
};

#endif // LOCALIZATION_MAPPER