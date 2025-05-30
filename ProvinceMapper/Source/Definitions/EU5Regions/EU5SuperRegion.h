#ifndef EU5_SUPERREGION_H
#define EU5_SUPERREGION_H
#include "EU5Region.h"

namespace EU5
{
class EU5SuperRegion: commonItems::parser
{
  public:
	EU5SuperRegion() = default;
	explicit EU5SuperRegion(std::istream& theStream);

	[[nodiscard]] const auto& getRegions() const { return regions; }
	[[nodiscard]] bool superRegionContainsLocation(const std::string& location) const;

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<EU5Region>> regions;
};
} // namespace EU5

#endif // EU4_SUPERREGION_H