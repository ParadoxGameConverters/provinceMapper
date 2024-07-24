#ifndef LINK_BASE_H
#define LINK_BASE_H
#include "Parser.h"
class wxColour;
class LinkBase
{
  public:
	explicit LinkBase(int theID);

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getComment() const { return comment; }
	[[nodiscard]] virtual const std::string toRowString();
	[[nodiscard]] virtual const wxColour getBaseRowColour();
	[[nodiscard]] virtual const wxColour getActiveRowColour();

	void setComment(const std::string& theComment) { comment = theComment; }

  protected:
	int ID = 0;
	std::optional<std::string> comment;
};

#endif // LINK_BASE_H
