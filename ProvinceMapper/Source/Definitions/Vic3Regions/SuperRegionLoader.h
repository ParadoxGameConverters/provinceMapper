#ifndef SUPERREGION_LOADER_H
#define SUPERREGION_LOADER_H
#include <Parser.h>

namespace V3
{
class SuperRegion;
class SuperRegionLoader: commonItems::parser
{
  public:
	SuperRegionLoader() = default;

	void loadSuperRegions(const std::filesystem::path& folderPath);
	[[nodiscard]] const auto& getSuperRegions() const { return superRegions; }
	[[nodiscard]] std::optional<std::string> getParentRegionName(const std::string& stateName) const;
	[[nodiscard]] std::optional<std::string> getParentSuperRegionName(const std::string& stateName) const;

  private:
	std::map<std::string, std::shared_ptr<SuperRegion>> superRegions;
};
} // namespace V3
#endif // SUPERREGION_LOADER_H