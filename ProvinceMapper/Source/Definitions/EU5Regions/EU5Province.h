#ifndef EU5_PROVINCE_H
#define EU5_PROVINCE_H
#include <Parser.h>
#include <set>

namespace EU5
{
class EU5Province
{
  public:
	EU5Province() = default;
	explicit EU5Province(const std::vector<std::string>& theLocations);

	[[nodiscard]] const auto& getLocations() const { return locations; }
	[[nodiscard]] bool provinceContainsLocation(const std::string& location) const;

  private:
	std::set<std::string> locations;
};
} // namespace EU5

#endif // EU5_PROVINCE_H