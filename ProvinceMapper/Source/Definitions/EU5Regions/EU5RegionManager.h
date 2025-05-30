#ifndef EU5_REGIONMANAGER_H
#define EU5_REGIONMANAGER_H
#include "EU5Continent.h"
#include <Parser.h>

namespace EU5
{
class EU5RegionManager: commonItems::parser
{
  public:
	void loadContinents(const std::filesystem::path& EU5Path);

	[[nodiscard]] std::optional<std::string> getParentProvinceName(const std::string& location) const;
	[[nodiscard]] std::optional<std::string> getParentAreaName(const std::string& location) const;
	[[nodiscard]] std::optional<std::string> getParentRegionName(const std::string& location) const;
	[[nodiscard]] std::optional<std::string> getParentSuperRegionName(const std::string& location) const;
	[[nodiscard]] std::optional<std::string> getParentContinentName(const std::string& location) const;

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<EU5Continent>> continents;
};
} // namespace EU5

#endif // EU5_REGIONMANAGER_H
