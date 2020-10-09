#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H

#include <set>

#include "Parser.h"
#include "Province.h"

class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream);

	void setName(const std::string& theName) { name = theName; }
	void toggleSource(const Province& theSource);
	void toggleTarget(const Province& theTarget);
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] const auto& getName() const { return name; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys();

	std::string name;
	std::set<Province> sources;
	std::set<Province> targets;
	std::string comment;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
