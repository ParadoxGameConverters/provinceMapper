#ifndef V3_SUPERREGION_H
#define V3_SUPERREGION_H
#include <Parser.h>

namespace V3
{
class Region;
class SuperRegion: commonItems::parser
{
  public:
	SuperRegion() = default;

	void initializeSuperRegion(std::istream& theStream);
	[[nodiscard]] const auto& getRegions() const { return regions; }
	[[nodiscard]] bool containsRegion(const std::string& region) const { return regions.contains(region); }

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<Region>> regions;
};
} // namespace V3
#endif // V3_SUPERREGION_H