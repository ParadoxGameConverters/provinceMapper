#ifndef LINK_MAPPING_H
#define LINK_MAPPING_H
#include "Definitions/DefinitionsInterface.h"
#include "Parser.h"

enum class Mapping
{
	MAPPED,
	UNMAPPED,
	FAIL
};
struct Province;
class LinkMapping: commonItems::parser
{
  public:
	LinkMapping() = default;
	explicit LinkMapping(std::istream& theStream,
		 std::shared_ptr<DefinitionsInterface> theSourceDefs,
		 std::shared_ptr<DefinitionsInterface> theTargetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);
	explicit LinkMapping(std::shared_ptr<DefinitionsInterface> theSourceDefs,
		 std::shared_ptr<DefinitionsInterface> theTargetDefs,
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);

	bool operator==(const LinkMapping& rhs) const;

	[[nodiscard]] Mapping toggleSource(const std::string& sourceID);
	[[nodiscard]] Mapping toggleTarget(const std::string& targetID);

	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	friend std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);

  private:
	void registerKeys();

	int ID = 0;
	std::vector<std::shared_ptr<Province>> sources;
	std::vector<std::shared_ptr<Province>> targets;
	std::optional<std::string> comment;
	std::shared_ptr<DefinitionsInterface> sourceDefs;
	std::shared_ptr<DefinitionsInterface> targetDefs;
	std::string sourceToken;
	std::string targetToken;
};
std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping);
#endif // LINK_MAPPING_H
