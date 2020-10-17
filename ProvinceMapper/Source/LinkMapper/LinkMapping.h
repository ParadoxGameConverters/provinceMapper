#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H
#include "Parser.h"

struct Province;
class Definitions;
class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream,
		 std::shared_ptr<Definitions> theSourceDefs,
		 std::shared_ptr<Definitions> theTargetDefs,
		 const std::string& sourceToken,
		 const std::string& targetToken,
		 int theID);
	explicit LinkMapping(std::shared_ptr<Definitions> theSourceDefs, std::shared_ptr<Definitions> theTargetDefs, int theID);

	bool operator==(const LinkMapping& rhs) const;

	void toggleSource(int sourceID);
	void toggleTarget(int targetID);
	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys(const std::string& sourceToken, const std::string& targetToken);

	int ID = 0;
	std::vector<std::shared_ptr<Province>> sources;
	std::vector<std::shared_ptr<Province>> targets;
	std::optional<std::string> comment;
	std::shared_ptr<Definitions> sourceDefs;
	std::shared_ptr<Definitions> targetDefs;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
