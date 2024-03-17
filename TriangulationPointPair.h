#ifndef TRIANGULATION_POINT_PAIR_H
#define TRIANGULATION_POINT_PAIR_H
#include "Definitions/DefinitionsInterface.h"
#include "Parser.h"

enum class Mapping
{
	MAPPED,
	UNMAPPED,
	FAIL
};
struct Province;
class TriangulationPointPair: commonItems::parser
{
  public:
	TriangulationPointPair() = default;
	explicit TriangulationPointPair(
		 std::istream& theStream,
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);
	explicit TriangulationPointPair(
		 std::string theSourceToken,
		 std::string theTargetToken,
		 int theID);

	bool operator==(const TriangulationPointPair& rhs) const;

	[[nodiscard]] Mapping toggleSource(const std::string& sourceID); // TODO: check if this can be removed
	[[nodiscard]] Mapping toggleTarget(const std::string& targetID);

	void setComment(const std::string& theComment) { comment = theComment; }

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getSources() const { return sources; }
	[[nodiscard]] const auto& getTargets() const { return targets; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	friend std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);

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
std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair);
#endif // TRIANGULATION_POINT_PAIR_H
