#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H

#include <set>
#include "Parser.h"

class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream);

	void setName(const std::string& theName) { name = theName; }
	void toggleSource(int theSource);
	void toggleTarget(int theTarget);
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] const auto& getName() const { return name; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys();

	std::string name;
	std::set<int> sources;
	std::set<int> targets;
	std::string comment;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
