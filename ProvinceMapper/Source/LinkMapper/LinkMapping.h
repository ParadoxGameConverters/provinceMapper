#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H
#include "Parser.h"

class Province;
class Definitions;
class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream,
		 const Definitions& sourceDefs,
		 const Definitions& targetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken,
		 int theID);

	bool operator==(const LinkMapping& rhs) const;

	void toggleSource(const std::shared_ptr<Province>& theSource);
	void toggleTarget(const std::shared_ptr<Province>& theTarget);
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs, const std::string& sourceToken, const std::string& targetToken);

	int ID = 0;
	std::vector<std::shared_ptr<Province>> sources;
	std::vector<std::shared_ptr<Province>> targets;
	std::string comment;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
