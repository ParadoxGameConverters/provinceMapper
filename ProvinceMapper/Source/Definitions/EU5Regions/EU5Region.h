#ifndef EU5_REGION_H
#define EU5_REGION_H
#include "EU5Area.h"
#include <Parser.h>

namespace EU5
{
class EU5Region: commonItems::parser
{
  public:
	EU5Region() = default;
	explicit EU5Region(std::istream& theStream);

	[[nodiscard]] const auto& getAreas() const { return areas; }
	[[nodiscard]] bool regionContainsLocation(const std::string& location) const;

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<EU5Area>> areas;
};
} // namespace EU5

#endif // EU5_REGION_H