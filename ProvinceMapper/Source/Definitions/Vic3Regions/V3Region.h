#ifndef V3_REGION_H
#define V3_REGION_H
#include "Parser.h"

namespace V3
{
class Region: commonItems::parser
{
  public:
	Region() = default;

	void initializeRegion(std::istream& theStream);
	[[nodiscard]] const auto& getStates() const { return states; }
	[[nodiscard]] bool containsState(const std::string& state) const { return states.contains(state); }

  private:
	void registerKeys();

	std::set<std::string> states;
};
} // namespace V3
#endif // V3_REGION_H