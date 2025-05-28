#ifndef EU5_AREA_H
#define EU5_AREA_H
#include "EU5Province.h"
#include <Parser.h>

namespace EU5
{
class EU5Area: commonItems::parser
{
  public:
	EU5Area() = default;
	explicit EU5Area(std::istream& theStream);

	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] bool areaContainsLocation(const std::string& location) const;

	void linkProvince(const std::pair<std::string, std::shared_ptr<EU5Province>>& theProvince) { provinces.at(theProvince.first) = theProvince.second; }

  private:
	void registerKeys();

	std::map<std::string, std::shared_ptr<EU5Province>> provinces;
};
} // namespace EU5

#endif // EU5_AREA_H