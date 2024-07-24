#ifndef LINK_BASE_H
#define LINK_BASE_H
#include "Definitions/DefinitionsInterface.h"
#include "Parser.h"
class LinkBase
{
  public:
	explicit LinkBase(int theID);

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getComment() const { return comment; }

	void setComment(const std::string& theComment) { comment = theComment; }

  protected:
	int ID = 0;
	std::optional<std::string> comment;
};

#endif // LINK_BASE_H
