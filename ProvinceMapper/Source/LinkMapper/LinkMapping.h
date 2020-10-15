#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H

#include "Parser.h"

class Province;
class Definitions;
class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream, const Definitions& sourceDefs, const Definitions& targetDefs);

	void toggleSource(const std::shared_ptr<Province>& theSource);
	void toggleTarget(const std::shared_ptr<Province>& theTarget);
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] const auto& getComment() const { return comment; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs);

	std::vector<std::shared_ptr<Province>> sources;
	std::vector<std::shared_ptr<Province>> targets;
	std::string comment;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
